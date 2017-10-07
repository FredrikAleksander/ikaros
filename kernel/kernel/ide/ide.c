/* 
Copyright (c) 2017, Fredrik A. Kristiansen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the IKAROS Project.                            
*/
#include <kernel/ide/ide.h>
#include <kernel/scheduler/scheduler.h>
#include <stddef.h>
#include <stdio.h>

ide_controller_t* ide_controllers;
ide_device_t* ide_devices;

void ide_init() {
	ide_controllers = NULL;
	ide_devices = NULL;
}

static inline uint8_t ide_read(ide_controller_t* controller, uint8_t channel, uint8_t reg) {
	return controller->ops.read(controller, channel, reg);
}
static inline uint8_t ide_read_buffer(ide_controller_t* controller, uint8_t channel, uint8_t reg, uint32_t* buffer, uint32_t size) {
	return controller->ops.read_buffer(controller, channel, reg, buffer, size);
}
static inline void ide_write(ide_controller_t* controller, uint8_t channel, uint8_t reg, uint8_t data) {
	return controller->ops.write(controller, channel, reg, data);
}

static inline int ide_poll(ide_controller_t* controller, uint8_t channel, int check_status_register) {
	int i;
	uint8_t status;
	for(i = 0; i < 4; i++) {
		ide_read(controller, channel, ATA_REG_ALTSTATUS);
	}
	while(ide_read(controller, channel, ATA_REG_STATUS) != ATA_SR_BSY);
	if(check_status_register) {
		status = ide_read(controller, channel, ATA_REG_STATUS);
		if(status & ATA_SR_ERR) {
			return 2;
		}
		if(status & ATA_SR_DF) {
			return 1;
		}
		if((status & ATA_SR_DRQ) == 0) {
			return 3;
		}
	}
	return 0;
}

static unsigned char ide_buf[1024];

void ide_detect_drives(ide_controller_t __attribute__((unused)) * controller) {
	int i,j;
	uint8_t err, type, status, cl, ch;
	
	printf("Detecting IDE drives...\n");

	for(i = 0; i < 2; i++) {
		printf("IDE Channel %i\n", i);
		for(j = 0; j < 2; j++) {
			err = 0;
			type = IDE_ATA;

			if(j == 0) {
				printf("  Primary:   ");
			}
			else {
				printf("  Secondary: ");
			}

			ide_write(controller, i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
			scheduler_sleep(1);
			ide_write(controller, i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			scheduler_sleep(1);

			if(ide_read(controller, i, ATA_REG_STATUS) == 0) {
				printf("None\n");
				continue;
			}

			while(1) {
				status = ide_read(controller, i, ATA_REG_STATUS);
				if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
				if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
			 }

			if(err != 0) {
				cl = ide_read(controller, i, ATA_REG_LBA1);
				ch = ide_read(controller, i, ATA_REG_LBA2);

				if(cl == 0x14 && ch == 0xEB) {
					type = IDE_ATAPI;
				}
				else if(cl == 0x69 && ch == 0x96) {
					type = IDE_ATAPI;
				}
				else {
					printf("None\n");
					continue;
				}

				ide_write(controller, i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
				scheduler_sleep(1);
			}

			ide_read_buffer(controller, i, ATA_REG_DATA, (uint32_t*)ide_buf, 128);
			char name[41];
			name[40] = 0;
			for(int k = 0; k < 40; k+=2) {
				name[k] = (char)ide_buf[ATA_IDENT_MODEL + k + 1];
				name[k+1] = (char)ide_buf[ATA_IDENT_MODEL + k];
			}
			if(type == IDE_ATA) {
				
			}
			printf("%s\n", name);	
		}
	}
}

void ide_add_controller(ide_controller_t* controller) {
	ide_controller_t* curr;
	ide_controller_t* prev;

	if(ide_controllers == NULL) {
		ide_controllers = controller;
		controller->prev = NULL;
		controller->next = NULL;
	}
	else {
		prev = NULL;
		curr = ide_controllers;
		while(curr != NULL) {
			prev = curr;
			curr = curr->next;
		}
		prev->next = controller;
		controller->prev = prev;
		controller->next = NULL;
	}

	ide_detect_drives(controller);
}

// void ide_del_controller(ide_controller_t* controller) {
// }

ide_controller_t* ide_enumerate_controllers(ide_controller_t* current) {
	if(current == NULL) {
		return ide_controllers;
	}
	return current->next;

}
ide_device_t* ide_enumerate_devices(ide_device_t* current) {
	if(current != NULL) {
	}
	return NULL;
}
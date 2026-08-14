#include <bootloader/limine_requests.h>
#include <drivers/acpi.h>
#include <stdint.h>

#define P2V(phys) ((void*)((uint64_t)(phys) + hhdm_request.response->offset))

#define NULL ((void*)0)

uint16_t get_acpi_pm_timer_port(void) {
    if (rsdp_request.response == NULL || rsdp_request.response->address == NULL) {
        return 0;
    }

    acpi_rsdp_t* rsdp = (acpi_rsdp_t*)rsdp_request.response->address;

    if (rsdp->revision >= 2 && rsdp->xsdt_address != 0) {
        acpi_header_t* xsdt = (acpi_header_t*)P2V((uint64_t)rsdp->xsdt_address);
        
        uint32_t entries = (xsdt->length - sizeof(acpi_header_t)) / 8;
        uint64_t* pointers = (uint64_t*)((uint8_t*)xsdt + sizeof(acpi_header_t));

        for (uint32_t i = 0; i < entries; i++) {
            acpi_header_t* header = (acpi_header_t*)P2V((uint64_t)pointers[i]);
            
            if (header->signature[0] == 'F' && header->signature[1] == 'A' &&
                header->signature[2] == 'C' && header->signature[3] == 'P') {
                
                acpi_fadt_t* fadt = (acpi_fadt_t*)header;
                return (uint16_t)fadt->pm_timer_block;
            }
        }
    } else {
        acpi_header_t* rsdt = (acpi_header_t*)P2V((uint64_t)rsdp->rsdt_address);
        
        uint32_t entries = (rsdt->length - sizeof(acpi_header_t)) / 4;
        uint32_t* pointers = (uint32_t*)((uint8_t*)rsdt + sizeof(acpi_header_t));

        for (uint32_t i = 0; i < entries; i++) {
            acpi_header_t* header = (acpi_header_t*)P2V((uint64_t)pointers[i]);
            
            if (header->signature[0] == 'F' && header->signature[1] == 'A' &&
                header->signature[2] == 'C' && header->signature[3] == 'P') {
                
                acpi_fadt_t* fadt = (acpi_fadt_t*)header;
                return (uint16_t)fadt->pm_timer_block;
            }
        }
    }

    return 0;
}
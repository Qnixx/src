#ifndef ACPI_TABLES_H
#define ACPI_TABLES_H

#include <lib/types.h>
#include <lib/asm.h>


typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t rev;
    uint32_t rsdtaddr;
} _packed acpi_rsdp_t;


typedef struct {
    acpi_rsdp_t rsdp1;
    uint32_t length;
    uint64_t xsdtAddr;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} _packed acpi_rsdp2_t;


typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem[6];
    uint8_t rev;
    uint32_t rsdt;
    uint32_t length;
    uint64_t xsdt;
    uint8_t extended_checksum;
} _packed acpi_xsdp_t;


typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem[6];
    char oem_table[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} _packed acpi_header_t;


typedef struct {
    acpi_header_t header;
    uint32_t tables[];
} _packed acpi_rsdt_t;


typedef struct {
    acpi_header_t header;
    uint64_t tables[];
} _packed acpi_xsdt_t;


typedef struct {
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t base;
} _packed acpi_gas_t;


typedef struct {
    acpi_header_t header;
    uint32_t firmware_control;
    uint32_t dsdt; // pointer to dsdt

    uint8_t reserved;

    uint8_t profile;
    uint16_t sci_irq;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t cstate_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;

    // cmos registers
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    // ACPI 2.0 fields
    uint16_t iapc_boot_flags;
    uint8_t reserved2;
    uint32_t flags;

    acpi_gas_t reset_register;
    uint8_t reset_command;
    uint16_t arm_boot_flags;
    uint8_t minor_version;

    uint64_t x_firmware_control;
    uint64_t x_dsdt;

    acpi_gas_t x_pm1a_event_block;
    acpi_gas_t x_pm1b_event_block;
    acpi_gas_t x_pm1a_control_block;
    acpi_gas_t x_pm1b_control_block;
    acpi_gas_t x_pm2_control_block;
    acpi_gas_t x_pm_timer_block;
    acpi_gas_t x_gpe0_block;
    acpi_gas_t x_gpe1_block;
} _packed acpi_fadt_t;


typedef struct {
    acpi_header_t header;
    uint32_t lapic_addr;
    uint32_t flags;
} _packed acpi_madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
} _packed apic_header_t;

typedef struct {
    apic_header_t header;
    uint8_t processor_id;
    uint8_t apic_id;
    uint32_t flags;
} _packed local_apic_t;


typedef struct {
    apic_header_t header;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_addr;
    uint32_t global_system_interrupt_base;
} _packed io_apic_t;

typedef struct {
    apic_header_t header;
    uint8_t bus;
    uint8_t source;					// IRQ.
    uint32_t interrupt;				// GSI.
    uint16_t flags;
} _packed apic_interrupt_override_t;

#endif

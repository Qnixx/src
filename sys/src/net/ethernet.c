#include <drivers/net/rtl8139.h>
#include <net/ethernet.h>
#include <lib/module.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <lib/log.h>

MODULE("ethernet");

/*
 *  Ethernet Driver.
 *
 */

#define BIG_ENDIAN(v) ((((v) & 0xFF) << 8) | (((v) >> 8) & 0xFF))

void ethernet_send(mac_address_t dest, uint8_t* data, int length) {
	int size = length + sizeof(ethernet_header_t);
	ethernet_header_t* hdr = (ethernet_header_t*)kmalloc(sizeof(ethernet_header_t));

	if(dest) kmemcpy(hdr->dest, dest, sizeof(mac_address_t));
	else kmemset(hdr->dest, 0xFF, sizeof(mac_address_t));
	kmemcpy(hdr->src, rtl8139_mac_addr, sizeof(mac_address_t));
	hdr->ether_type = BIG_ENDIAN(ETHERTYPE_ARP);

	kmemcpy(hdr->payload, data, length);
	rtl8139_send_packet((uint8_t*)hdr, size);
}
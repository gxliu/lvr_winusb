#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libusb-1.0/libusb.h>

#define VERSION "0.1.0"
#define VENDOR_ID 0x0925
#define PRODUCT_ID 0x1456 
const static int PACKET_INT_LEN=8;
const static int PACKET_BULK_LEN=64;
const static int INTERFACE=0;
const static int ENDPOINT_INT_IN=0x82; /* endpoint 0x81 address for IN */
const static int ENDPOINT_INT_OUT=0x02; /* endpoint 1 address for OUT */
const static int ENDPOINT_BULK_IN=0x81; /* endpoint 0x81 address for IN */
const static int ENDPOINT_BULK_OUT=0x01; /* endpoint 1 address for OUT */
const static int TIMEOUT=5000; /* timeout in ms */

void bad(const char *why) {
	fprintf(stderr,"Fatal error> %s\n",why);
	exit(17);
}

static struct libusb_device_handle *devh = NULL;

static int find_lvr_winusb(void)
{
	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	return devh ? 0 : -EIO;
}

static int test_bulk_transfer(void)
{
   	int r,i;
	int transferred;
   	char answer[PACKET_BULK_LEN];
   	char question[PACKET_BULK_LEN];
   	for (i=0;i<PACKET_BULK_LEN; i++) question[i]=i;

	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_OUT, question, PACKET_BULK_LEN,
		&transferred,TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Bulk write error %d\n", r);
		return r;
	}
	r = libusb_bulk_transfer(devh, ENDPOINT_BULK_IN, answer,PACKET_BULK_LEN,
		&transferred, TIMEOUT);
	if (r < 0) {
		fprintf(stderr, "Bulk read error %d\n", r);
		return r;
	}
	if (transferred < PACKET_BULK_LEN) {
		fprintf(stderr, "Bulk transfer short read (%d)\n", r);
		return -1;
	}

//   	for (i=0;i< PACKET_BULK_LEN;i++) printf("%i, %i,\n ",question[i],answer[i]);
	for(i = 0;i < PACKET_BULK_LEN; i++) {
		if(i%8 == 0)
			printf("\n");
		printf("%02x, %02x; ",question[i],answer[i]);
	}
	printf("\n");

	return 0;
}

int main(void)
{
	struct sigaction sigact;
	int r = 1;

	r = libusb_init(NULL);
	if (r < 0) {
		fprintf(stderr, "failed to initialise libusb\n");
		exit(1);
	}

	r = find_lvr_winusb();
	if (r < 0) {
		fprintf(stderr, "Could not find/open device\n");
		goto out;
	}
	printf("Successfully find the LVR WINUSB device\n");
	r = libusb_set_configuration(devh, 1);
	if (r < 0) {
		fprintf(stderr, "libusb_set_configuration error %d\n", r);
		goto out;
	}
	printf("Successfully set usb configuration 1\n");
	r = libusb_claim_interface(devh, 0);
	if (r < 0) {
		fprintf(stderr, "libusb_claim_interface error %d\n", r);
		goto out;
	}
	printf("Successfully claimed interface\n");
	test_bulk_transfer();
	libusb_release_interface(devh, 0);
out:
	libusb_reset_device(devh);
	libusb_close(devh);
	libusb_exit(NULL);
	return r >= 0 ? r : -r;
}

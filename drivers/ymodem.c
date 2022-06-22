#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"

//#include "my_type.h"
#include "ymodem.h"
#include "comfunc.h"
#include "uart.h"
#include "dbug.h"


#if CONFIG_DEBUG

static int Ymodem_receive_packet (char *data, int *length, int timeout)
{
	char c;
	int i, packet_size;
	uint16_t crc, crc_rcvd;

	*length = 0;

	if (board_getchar_timeout(&c, timeout) != 0)
		return -1;

	switch (c) {
	case SOH:
		packet_size = PACKET_SIZE;
		break;
	case STX:
		packet_size = PACKET_1K_SIZE;
		break;
	case EOT:
		return 0;
	case CA:
		if ((board_getchar_timeout(&c, timeout) == 0) && (c == CA)) {
			*length = -2;
			return 0;
		}
		return -3;
	case 0x03:
	case ABORT1:
	case ABORT2:
		return 1;
	default:
		return -4;
	}
	*data = c;
	/*接收数据*/
	for (i = 1; i < (packet_size + PACKET_OVERHEAD); i ++) {
		if (board_getchar_timeout(data + i, timeout) != 0)
			return -5;
	}
	/*校验序号*/
	if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
		return -6;
	crc = Cal_CRC16((const uint8_t*)data+PACKET_HEADER, packet_size);
	crc_rcvd = (data[PACKET_HEADER+packet_size] << 8) | ((unsigned char)(data[PACKET_HEADER+packet_size+1]));
	if (crc != crc_rcvd) 
		return -7;
	*length = packet_size;
	return 0;
}


int Ymodem_receive(int app_addr, int max_size)
{
	char packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH], *file_ptr;
	int i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
	int ret;

	board_putchar(CRC16);
	for (session_done = 0, errors = 0, session_begin = 0; ;) 
	{
		for (packets_received = 0, file_done = 0; ;) 
		{
			ret = Ymodem_receive_packet(packet_data, &packet_length, NAK_TIMEOUT);
			switch (ret) {
			case 0:
				errors = 0;
				switch (packet_length) {
				/* Abort by sender */
				case - 1:
					board_putchar(ACK);
					return 0;
					/* End of transmission */
				case 0:
					board_putchar(ACK);
					board_putchar(CRC16);
					file_done = 1;
					break;
					/* Normal packet */
				default:
					if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff)) {
						board_putchar(NAK);
					}
					else {
						if (packets_received == 0) {
							/* Filename packet */
							if (packet_data[PACKET_HEADER] != 0) {
								/* Filename packet has valid data */
								file_ptr = packet_data + PACKET_HEADER;
								file_ptr += strlen(file_ptr);
								for (i = 0, file_ptr ++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH);)
									file_size[i++] = *file_ptr++;
								file_size[i++] = '\0';
								size = strtoul((char *)file_size, NULL, 10);
								if (size > max_size) {
									board_putchar(CA);
									board_putchar(CA);
									return -1;
								}
								dataflash_erase(app_addr, size);
								board_putchar(ACK);
								board_putchar(CRC16);
							}
							/* Filename packet is empty, end session */
							else {
								board_putchar(ACK);
								file_done = 1;
								session_done = 1;

								break;
							}
						}
						/* Data packet */
						else {
							int prog_len;
                            prog_len = packet_length;
							dataflash_write(app_addr, packet_data + PACKET_HEADER, prog_len);
							app_addr += prog_len;
							board_putchar(ACK);
						}
						++packets_received;
						session_begin = 1;
					}
				}
				break;
			case 1:
				board_putchar(CA);
				board_putchar(CA);
				return -3;
			default:
				if (session_begin > 0) {
					errors ++;
				}
				if (errors > MAX_ERRORS) {
					board_putchar(CA);
					board_putchar(CA);
					printD("error exeed max count %d",ret);
					return 0;
				}
				board_putchar(CRC16);
				break;
			}
			if (file_done != 0) {
				break;
			}
		}
		if (session_done != 0) {
			break;
		}
	}
	return size;
}

#endif 

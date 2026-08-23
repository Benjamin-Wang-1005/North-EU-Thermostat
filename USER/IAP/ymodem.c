#include "iap_flash.h"
#include "iap_uart.h"
#include "ymodem.h"
#include "string.h"
#include "menu.h"

uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
extern uint8_t aFileName[FILE_NAME_LENGTH];

static void PrepareIntialPacket(uint8_t *p_data, const uint8_t *p_file_name, uint32_t length)
{
    uint32_t i, j = 0;
    uint8_t astring[10];

    p_data[PACKET_START_INDEX] = SOH;
    p_data[PACKET_NUMBER_INDEX] = 0x00;
    p_data[PACKET_CNUMBER_INDEX] = 0xff;

    for (i = 0; (p_file_name[i] != 0) && (i < FILE_NAME_LENGTH); i++)
    {
        p_data[i + PACKET_DATA_INDEX] = p_file_name[i];
    }

    p_data[i + PACKET_DATA_INDEX] = 0x00;

    Int2Str(astring, length);
    i = i + PACKET_DATA_INDEX + 1;
    while (astring[j] != 0)
    {
        p_data[i++] = astring[j++];
    }

    for (j = i; j < PACKET_SIZE + PACKET_DATA_INDEX; j++)
    {
        p_data[j] = 0;
    }
}

static void PreparePacket(uint8_t *p_source, uint8_t *p_packet, uint8_t pkt_nr, uint32_t size_blk)
{
    uint8_t *p_record;
    uint32_t i, size, packet_size;

    packet_size = size_blk >= PACKET_1K_SIZE ? PACKET_1K_SIZE : PACKET_SIZE;
    size = size_blk < packet_size ? size_blk : packet_size;
    if (packet_size == PACKET_1K_SIZE)
    {
        p_packet[PACKET_START_INDEX] = STX;
    }
    else
    {
        p_packet[PACKET_START_INDEX] = SOH;
    }
    p_packet[PACKET_NUMBER_INDEX] = pkt_nr;
    p_packet[PACKET_CNUMBER_INDEX] = (~pkt_nr);
    p_record = p_source;

    for (i = PACKET_DATA_INDEX; i < size + PACKET_DATA_INDEX; i++)
    {
        p_packet[i] = *p_record++;
    }
    if (size <= packet_size)
    {
        for (i = size + PACKET_DATA_INDEX; i < packet_size + PACKET_DATA_INDEX; i++)
        {
            p_packet[i] = 0x1A;
        }
    }
}

uint16_t UpdateCRC16(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;

    do
    {
        crc <<= 1;
        in <<= 1;
        if (in & 0x100)
            ++crc;
        if (crc & 0x10000)
            crc ^= 0x1021;
    } while (!(in & 0x10000));

    return crc & 0xffffu;
}

uint16_t Cal_CRC16(const uint8_t* p_data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = p_data + size;

    while (p_data < dataEnd)
        crc = UpdateCRC16(crc, *p_data++);

    crc = UpdateCRC16(crc, 0);
    crc = UpdateCRC16(crc, 0);

    return crc & 0xffffu;
}

uint8_t CalcChecksum(const uint8_t *p_data, uint32_t size)
{
    uint32_t sum = 0;
    const uint8_t *p_data_end = p_data + size;

    while (p_data < p_data_end)
    {
        sum += *p_data++;
    }

    return (sum & 0xffu);
}

static uint32_t ReceivePacket(uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
    uint32_t crc;
    uint32_t packet_size = 0;
    uint32_t status;
    uint8_t char1;

    *p_length = 0;
    status = Serial_GetByte(&char1, timeout);

    if (status == 0)
    {
        switch (char1)
        {
            case SOH:
                packet_size = PACKET_SIZE;
                break;
            case STX:
                packet_size = PACKET_1K_SIZE;
                break;
            case EOT:
                break;
            case CA:
                if ((Serial_GetByte(&char1, timeout) == 0) && (char1 == CA))
                {
                    packet_size = 2;
                }
                else
                {
                    status = 1;
                }
                break;
            case ABORT1:
            case ABORT2:
                status = 2;
                break;
            default:
                status = 1;
                break;
        }
        *p_data = char1;

        if (packet_size >= PACKET_SIZE)
        {
            uint32_t i;
            for (i = 0; i < packet_size + PACKET_OVERHEAD_SIZE; i++)
            {
                if (Serial_GetByte(&p_data[PACKET_NUMBER_INDEX + i], timeout) != 0)
                {
                    packet_size = 0;
                    status = 1;
                    break;
                }
            }

            if (status == 0)
            {
                if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
                {
                    packet_size = 0;
                    status = 1;
                }
                else
                {
                    crc = p_data[packet_size + PACKET_DATA_INDEX] << 8;
                    crc += p_data[packet_size + PACKET_DATA_INDEX + 1];
                    if (Cal_CRC16(&p_data[PACKET_DATA_INDEX], packet_size) != crc)
                    {
                        packet_size = 0;
                        status = 1;
                    }
                }
            }
            else
            {
                packet_size = 0;
            }
        }
    }
    *p_length = packet_size;
    return status;
}

COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size)
{
    uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
    uint32_t flashdestination, ramsource, filesize;
    uint8_t *file_ptr;
    uint8_t file_size[FILE_SIZE_LENGTH], tmp, packets_received;
    COM_StatusTypeDef result = COM_OK;

    flashdestination = APPLICATION_ADDRESS;

    Serial_PutByte(CRC16);

    while ((session_done == 0) && (result == COM_OK))
    {
        packets_received = 0;
        file_done = 0;

        while ((file_done == 0) && (result == COM_OK))
        {
            switch (ReceivePacket(aPacketData, &packet_length, DOWNLOAD_TIMEOUT))
            {
                case 0:
                    errors = 0;
                    switch (packet_length)
                    {
                        case 2:
                            Serial_PutByte(ACK);
                            result = COM_ABORT;
                            break;
                        case 0:
                            Serial_PutByte(ACK);
                            file_done = 1;
                            break;
                        default:
                            if (aPacketData[PACKET_NUMBER_INDEX] == packets_received - 1)
                            {
                                /* Duplicate packet -- sender retransmitted, just ACK */
                                Serial_PutByte(ACK);
                            }
                            else if (aPacketData[PACKET_NUMBER_INDEX] != packets_received)
                            {
                                Serial_PutByte(NAK);
                            }
                            else
                            {
                                if (packets_received == 0)
                                {
                                    if (aPacketData[PACKET_DATA_INDEX] != 0)
                                    {
                                        i = 0;
                                        file_ptr = aPacketData + PACKET_DATA_INDEX;
                                        while ((*file_ptr != 0) && (i < FILE_NAME_LENGTH))
                                        {
                                            aFileName[i++] = *file_ptr++;
                                        }
                                        aFileName[i++] = 0;
                                        i = 0;
                                        file_ptr++;
                                        while ((*file_ptr != 0x20) && (i < FILE_SIZE_LENGTH))
                                        {
                                            file_size[i++] = *file_ptr++;
                                        }
                                        file_size[i++] = 0;
                                        Str2Int(file_size, &filesize);

                                        if (*p_size > (USER_FLASH_SIZE + 1))
                                        {
                                            tmp = CA;
                                            Serial_PutByte(tmp);
                                            Serial_PutByte(tmp);
                                            result = COM_LIMIT;
                                        }
                                        *p_size = filesize;

                                        Serial_PutByte(ACK);
                                        Serial_PutByte(CRC16);
                                    }
                                    else
                                    {
                                        Serial_PutByte(ACK);
                                        file_done = 1;
                                        session_done = 1;
                                        break;
                                    }
                                }
                                else
                                {
                                    ramsource = (uint32_t) &aPacketData[PACKET_DATA_INDEX];

                                    if (FLASH_If_Write(flashdestination, (uint32_t*)ramsource, packet_length / 4) == FLASHIF_OK)
                                    {
                                        flashdestination += packet_length;
                                        Serial_PutByte(ACK);
                                    }
                                    else
                                    {
                                        Serial_PutByte(CA);
                                        Serial_PutByte(CA);
                                        result = COM_DATA;
                                    }
                                }
                                packets_received++;
                                session_begin = 1;
                            }
                            break;
                    }
                    break;
                case 2:
                    Serial_PutByte(CA);
                    Serial_PutByte(CA);
                    result = COM_ABORT;
                    break;
                default:
                    if (session_begin > 0)
                    {
                        errors++;
                        if (errors > MAX_ERRORS)
                        {
                            Serial_PutByte(CA);
                            Serial_PutByte(CA);
                        }
                        else
                        {
                            Serial_PutByte(CRC16);
                        }
                    }
                    else
                    {
                        Serial_PutByte(CRC16);
                    }
                    break;
            }
        }
    }
    return result;
}

COM_StatusTypeDef Ymodem_Transmit(uint8_t *p_buf, const uint8_t *p_file_name, uint32_t file_size)
{
    uint32_t errors = 0, ack_recpt = 0, size = 0, pkt_size;
    uint8_t *p_buf_int;
    COM_StatusTypeDef result = COM_OK;
    uint32_t blk_number = 1;
    uint8_t a_rx_ctrl[2];
    uint8_t i;
    uint32_t temp_crc;

    PrepareIntialPacket(aPacketData, p_file_name, file_size);

    while ((!ack_recpt) && (result == COM_OK))
    {
        for (i = PACKET_START_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
        {
            Serial_PutByte(aPacketData[i]);
        }
        temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        Serial_PutByte(temp_crc >> 8);
        Serial_PutByte(temp_crc & 0xFF);

        if (Serial_GetByte(&a_rx_ctrl[0], NAK_TIMEOUT) == 0)
        {
            if (a_rx_ctrl[0] == ACK)
            {
                ack_recpt = 1;
            }
            else if (a_rx_ctrl[0] == CA)
            {
                if ((Serial_GetByte(&a_rx_ctrl[0], NAK_TIMEOUT) == 0) && (a_rx_ctrl[0] == CA))
                {
                    Serial_Flush();
                    result = COM_ABORT;
                }
            }
        }
        else
        {
            errors++;
        }
        if (errors >= MAX_ERRORS)
        {
            result = COM_ERROR;
        }
    }

    p_buf_int = p_buf;
    size = file_size;

    while ((size) && (result == COM_OK))
    {
        PreparePacket(p_buf_int, aPacketData, blk_number, size);
        ack_recpt = 0;
        a_rx_ctrl[0] = 0;
        errors = 0;

        while ((!ack_recpt) && (result == COM_OK))
        {
            if (size >= PACKET_1K_SIZE)
            {
                pkt_size = PACKET_1K_SIZE;
            }
            else
            {
                pkt_size = PACKET_SIZE;
            }

            for (i = PACKET_START_INDEX; i < pkt_size + PACKET_DATA_INDEX; i++)
            {
                Serial_PutByte(aPacketData[i]);
            }

            temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], pkt_size);
            Serial_PutByte(temp_crc >> 8);
            Serial_PutByte(temp_crc & 0xFF);

            if ((Serial_GetByte(&a_rx_ctrl[0], NAK_TIMEOUT) == 0) && (a_rx_ctrl[0] == ACK))
            {
                ack_recpt = 1;
                if (size > pkt_size)
                {
                    p_buf_int += pkt_size;
                    size -= pkt_size;
                    if (blk_number == (USER_FLASH_SIZE / PACKET_1K_SIZE))
                    {
                        result = COM_LIMIT;
                    }
                    else
                    {
                        blk_number++;
                    }
                }
                else
                {
                    p_buf_int += pkt_size;
                    size = 0;
                }
            }
            else
            {
                errors++;
            }

            if (errors >= MAX_ERRORS)
            {
                result = COM_ERROR;
            }
        }
    }

    ack_recpt = 0;
    a_rx_ctrl[0] = 0x00;
    errors = 0;
    while ((!ack_recpt) && (result == COM_OK))
    {
        Serial_PutByte(EOT);

        if (Serial_GetByte(&a_rx_ctrl[0], NAK_TIMEOUT) == 0)
        {
            if (a_rx_ctrl[0] == ACK)
            {
                ack_recpt = 1;
            }
            else if (a_rx_ctrl[0] == CA)
            {
                if ((Serial_GetByte(&a_rx_ctrl[0], NAK_TIMEOUT) == 0) && (a_rx_ctrl[0] == CA))
                {
                    Serial_Flush();
                    result = COM_ABORT;
                }
            }
        }
        else
        {
            errors++;
        }

        if (errors >= MAX_ERRORS)
        {
            result = COM_ERROR;
        }
    }

    if (result == COM_OK)
    {
        aPacketData[PACKET_START_INDEX] = SOH;
        aPacketData[PACKET_NUMBER_INDEX] = 0;
        aPacketData[PACKET_CNUMBER_INDEX] = 0xFF;
        for (i = PACKET_DATA_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
        {
            aPacketData[i] = 0x00;
        }

        for (i = PACKET_START_INDEX; i < (PACKET_SIZE + PACKET_DATA_INDEX); i++)
        {
            Serial_PutByte(aPacketData[i]);
        }

        temp_crc = Cal_CRC16(&aPacketData[PACKET_DATA_INDEX], PACKET_SIZE);
        Serial_PutByte(temp_crc >> 8);
        Serial_PutByte(temp_crc & 0xFF);

        if (Serial_GetByte(&a_rx_ctrl[0], NAK_TIMEOUT) == 0)
        {
            if (a_rx_ctrl[0] == CA)
            {
                Serial_Flush();
                result = COM_ABORT;
            }
        }
    }

    return result;
}

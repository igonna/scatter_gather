////////////////////////////////////////////////////////////////////////////////
//
//  File           : sg_driver.c
//  Description    : This file contains the driver code to be developed by
//                   the students of the 311 class.  See assignment details
//                   for additional information.
//
//   Author        : ????
//   Last Modified : ????
//

// Include Files
#include <string.h>
#include <stdlib.h>
// Project Includes 
#include <sg_driver.h>

// Defines

/*typedef struct {
	uint32_t magic_v1;
	SG_Node_ID locNodeId;
	SG_Node_ID remNodeId;
	SG_Block_ID blockID;
	SG_System_OP operation;
	SG_SeqNum sendSeqNo;
	SG_SeqNum recvSeqNo;
	char indc_data;
//	SG_Data_Block *data;
	uint32_t magic_v2;
}SG_Packet_WD;*/
//
// Global Data
typedef struct {
    uint32_t		magic_v1;
    SG_Node_ID		locNodeId;
    SG_Node_ID		remNodeId;
    SG_Block_ID		blockID;
    SG_System_OP	operation;
    SG_SeqNum		sendSeqNo;
    SG_SeqNum		recvSeqNo;
    char		indc_data;
    uint32_t		magic_v2;
}SG_Packet_ND;

typedef struct {
    uint32_t		magic_v1;
    SG_Node_ID		locNodeId;
    SG_Node_ID		remNodeId;
    SG_Block_ID		blockID;
    SG_System_OP	operation;
    SG_SeqNum		sendSeqNo;
    SG_SeqNum		recvSeqNo;
    char		indc_data;
    SG_Data_Block	*data;
    uint32_t		magic_v2;
}SG_Packet_WD;
//
// Functions

void sg_strcpy_t(SG_Data_Block *dblk, char *str)
{
	for (int i = 0; i < strlen(str) + 1; ++i)
		(*dblk)[i] = str[i];
}
void sg_strcpy_f(char *str, SG_Data_Block *dblk)
{
	for (int i = 0; i < SG_BLOCK_SIZE; ++i)
		str[i] = (*dblk)[i];
}

//from SG_Packet to char* 
void sg_pckt_buff(char *packet, void *pckt_wd, size_t sz)
{
    for (uint16_t step = 0; step < sz; ++step)
            packet[step] = ((char*)pckt_wd)[step];
}
void sg_strcpy(char *buff, char *sg_buff)
{
    for (uint16_t step = 0; step < SG_BLOCK_SIZE; ++step)
            if (sg_buff + step)
                    buff[step] = sg_buff[step];
            else
                    break;
}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : serialize_sg_packet
// Description  : Serialize a ScatterGather packet (create packet)
//
// Inputs       : loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
//                plen - the packet length (int bytes)
// Outputs      : 0 if successfully created, -1 if failure

SG_Packet_Status serialize_sg_packet( SG_Node_ID loc, SG_Node_ID rem, SG_Block_ID blk, 
        SG_System_OP op, SG_SeqNum sseq, SG_SeqNum rseq, char *data, 
	char *packet, size_t *plen )
{
//	printf("serialize:\n");
	SG_Packet_Status pack_status;
	pack_status = SG_PACKT_OK;

	SG_Packet_Info pack_info;
	//
	if (loc)
		pack_info.locNodeId = loc;
	else
		pack_status = SG_PACKT_LOCID_BAD;
	//
	if (rem)
		pack_info.remNodeId = rem;
	else
		pack_status = SG_PACKT_REMID_BAD;
	//
	if (blk)
		pack_info.blockID = blk;
	else
		pack_status = SG_PACKT_BLKID_BAD;
	//
	if (op >= SG_INIT_ENDPOINT && op <= SG_MAXVAL_OP)
		pack_info.operation = op;
	else
		pack_status = SG_PACKT_OPERN_BAD;
	//
	if (sseq)
		pack_info.sendSeqNo = sseq;
	else
		pack_status = SG_PACKT_SNDSQ_BAD;
	//
	if (rseq)
		pack_info.recvSeqNo = rseq;
	else
		pack_status = SG_PACKT_RCVSQ_BAD;
	//
	char data_indc;
	if (!data) {
		*plen = SG_DATA_PACKET_SIZE;
		data_indc = 0;

	} else {
		*plen = SG_BASE_PACKET_SIZE;
		data_indc = 1;
	}

//	printf("\t-sindc: %d\n", data_indc);

	SG_Packet_WD sg_packet_wd;
	SG_Packet_ND sg_packet_nd;
	if (data_indc) {
		sg_packet_wd.magic_v1 = SG_MAGIC_VALUE;
		sg_packet_wd.locNodeId = pack_info.locNodeId;
		sg_packet_wd.remNodeId = pack_info.remNodeId;
		sg_packet_wd.blockID = pack_info.blockID;
		sg_packet_wd.operation = pack_info.operation;
		sg_packet_wd.sendSeqNo = pack_info.sendSeqNo;
		sg_packet_wd.recvSeqNo = pack_info.recvSeqNo;
		sg_packet_wd.indc_data = data_indc;
		sg_packet_wd.magic_v2 = SG_MAGIC_VALUE;
//		printf("\tsmagic: %d\n", 
//			sg_packet_wd.magic_v2);
		sg_packet_wd.data = (SG_Data_Block*)data;
		*plen = sizeof(sg_packet_wd);//SG_DATA_PACKET_SIZE;
//		printf("size: %ld\n", *plen);
		sg_pckt_buff(packet, &sg_packet_wd, sizeof(sg_packet_wd));

printf("serialize:\nmagic1: %d\nlocNodeId: %ld\nremNodeId: %ld\nblockID: %ld\noperation: %d\nsendSeqNo: %d\nrecvSeqNo: %d\nindc: %d\nmagic2: %d\n\n",
    sg_packet_wd.magic_v1,
    sg_packet_wd.locNodeId,
    sg_packet_wd.remNodeId,
    sg_packet_wd.blockID,
    sg_packet_wd.operation,
    sg_packet_wd.sendSeqNo,
    sg_packet_wd.recvSeqNo,
    sg_packet_wd.indc_data,
    sg_packet_wd.magic_v2);
	} else {
		sg_packet_nd.magic_v1 = SG_MAGIC_VALUE;
		sg_packet_nd.locNodeId = pack_info.locNodeId;
		sg_packet_nd.remNodeId = pack_info.remNodeId;
		sg_packet_nd.blockID = pack_info.blockID;
		sg_packet_nd.operation = pack_info.operation;
		sg_packet_nd.sendSeqNo = pack_info.sendSeqNo;
		sg_packet_nd.recvSeqNo = pack_info.recvSeqNo;
		sg_packet_nd.indc_data = data_indc;
//		sg_packet_nd.data = NULL;
		sg_packet_nd.magic_v2 = SG_MAGIC_VALUE;
		*plen = sizeof(sg_packet_nd);//SG_BASE_PACKET_SIZE;
//		printf("size: %ld\n", *plen);
		sg_pckt_buff(packet, &sg_packet_nd, sizeof(sg_packet_nd));
		
//		printf("\tsmagic: %d\n", 
//			sg_packet_nd.magic_v2);
printf("serialize:\nmagic1: %d\nlocNodeId: %ld\nremNodeId: %ld\nblockID: %ld\noperation: %d\nsendSeqNo: %d\nrecvSeqNo: %d\nindc: %d\nmagic2: %d\n\n",
    sg_packet_nd.magic_v1,
    sg_packet_nd.locNodeId,
    sg_packet_nd.remNodeId,
    sg_packet_nd.blockID,
    sg_packet_nd.operation,
    sg_packet_nd.sendSeqNo,
    sg_packet_nd.recvSeqNo,
    sg_packet_nd.indc_data,
    sg_packet_nd.magic_v2);

	}
	return(pack_status); 
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : deserialize_sg_packet
// Description  : De-serialize a ScatterGather packet (unpack packet)
//
// Inputs       : loc - the local node identifier
//                rem - the remote node identifier
//                blk - the block identifier
//                op - the operation performed/to be performed on block
//                sseq - the sender sequence number
//                rseq - the receiver sequence number
//                data - the data block (of size SG_BLOCK_SIZE) or NULL
//                packet - the buffer to place the data
//                plen - the packet length (int bytes)
// Outputs      : 0 if successfully created, -1 if failure

SG_Packet_Status deserialize_sg_packet( SG_Node_ID *loc, SG_Node_ID *rem, SG_Block_ID *blk, 
        SG_System_OP *op, SG_SeqNum *sseq, SG_SeqNum *rseq, char *data, 
        char *packet, size_t plen )
{
	SG_Packet_Status pack_status;
	pack_status = SG_PACKT_OK;
	if (data == NULL)
		printf ("data null!------------------\n");
	char data_indc = -1;
	void *sg_packet = (void*)packet;
	char werr = 0;
	if (sizeof(SG_Packet_WD) == plen)
		data_indc = 1;
	if (sizeof(SG_Packet_ND) == plen)
		data_indc = 0;

	if (data_indc == -1) {
		if (!werr) {
			pack_status = SG_PACKT_BLKLN_BAD;
			werr = 1;
			return pack_status;
		}
	}
	if (((SG_Packet_ND*)sg_packet)->locNodeId)
		*loc = ((SG_Packet_ND*)sg_packet)->locNodeId;
	else {
		if (!werr) {
			pack_status = SG_PACKT_LOCID_BAD;
			werr = 1;
		}
	}
	if (((SG_Packet_ND*)sg_packet)->remNodeId)
		*rem = ((SG_Packet_ND*)sg_packet)->remNodeId;
	else
		if (!werr) {
			pack_status = SG_PACKT_REMID_BAD;
			werr = 1;
		}
	if (((SG_Packet_ND*)sg_packet)->blockID)
		*blk = ((SG_Packet_ND*)sg_packet)->blockID;
	else
		if (!werr) {
			pack_status = SG_PACKT_BLKID_BAD;
			werr = 1;
		}
	if (((SG_Packet_ND*)sg_packet)->operation >= SG_INIT_ENDPOINT
			&& ((SG_Packet_ND*)sg_packet)->operation <= SG_MAXVAL_OP)
		*op = ((SG_Packet_ND*)sg_packet)->operation;
	else
		if (!werr) {
			pack_status = SG_PACKT_OPERN_BAD;
			werr = 1;
		}
	if (((SG_Packet_ND*)sg_packet)->sendSeqNo)
		*sseq = ((SG_Packet_ND*)sg_packet)->sendSeqNo;
	else
		if (!werr) {
			pack_status = SG_PACKT_SNDSQ_BAD;
			werr = 1;
		}
	if (((SG_Packet_ND*)sg_packet)->recvSeqNo)
		*rseq = ((SG_Packet_ND*)sg_packet)->recvSeqNo;
	else
		if (!werr) {
			pack_status = SG_PACKT_RCVSQ_BAD;
			werr = 1;
		}
	if (data_indc == 1)
		strcpy(data, (char*)(((SG_Packet_WD*)sg_packet)->data));
	else
		data = NULL;
	printf("pack status: %d\n", pack_status);
printf("deserialize:\nmagic1: %d\nlocNodeId: %ld\nremNodeId: %ld\nblockID: %ld\noperation: %d\nsendSeqNo: %d\nrecvSeqNo: %d\nindc: %d\nmagic2: %d\n\n",
    ((SG_Packet_WD*)sg_packet)->magic_v1,
    *loc,
    *rem,
    *blk,
    *op,
    *sseq,
    *rseq,
    data_indc,
	((SG_Packet_WD*)sg_packet)->magic_v2);
	
	return(pack_status);
}






















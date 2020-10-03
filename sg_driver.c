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

//
// Functions

//from SG_Packet to char* 
void sg_pckt_buff(char *packet, void *pckt_wd, size_t sz)
{
    for (uint16_t step = 0; step < sz; ++step)
            packet[step] = ((char*)pckt_wd)[step];
}
void impose_buff(char *buff_to, char *buff_from, int begin, uint32_t end, uint32_t margin)
{
    for (; begin <= end; ++begin)
        buff_to[margin++] = buff_from[begin];
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
	if (data) {
		*plen = SG_DATA_PACKET_SIZE;
		data_indc = 1;

	} else {
		*plen = SG_BASE_PACKET_SIZE;
		data_indc = 0;
	}

	bzero(packet, SG_DATA_PACKET_SIZE);
	uint32_t packet_margin = 0;
	uint32_t magic_num = SG_MAGIC_VALUE;
	//magic
	strcpy(packet, (char*)&magic_num);
	packet_margin += sizeof(uint32_t);
	//locNodeId
	impose_buff(packet, (char*)&pack_info.locNodeId, 0, sizeof(SG_Node_ID) - 1, packet_margin);
	packet_margin += sizeof(SG_Node_ID);
	//remNodeId
	impose_buff(packet, (char*)&pack_info.remNodeId, 0, sizeof(SG_Node_ID) - 1, packet_margin);
	packet_margin += sizeof(SG_Node_ID);
	//blockID
	impose_buff(packet, (char*)&pack_info.blockID, 0, sizeof(SG_Block_ID) - 1, packet_margin);
	packet_margin += sizeof(SG_Block_ID);
	//operation
	impose_buff(packet, (char*)&pack_info.operation, 0, sizeof(SG_System_OP) - 1, packet_margin);
	packet_margin += sizeof(SG_System_OP);
	//sendSeqNo
	impose_buff(packet, (char*)&pack_info.sendSeqNo, 0, sizeof(SG_SeqNum) - 1, packet_margin);
	packet_margin += sizeof(SG_SeqNum);
	//recvSeqNo	
	impose_buff(packet, (char*)&pack_info.recvSeqNo, 0, sizeof(SG_SeqNum) - 1, packet_margin);
	packet_margin += sizeof(SG_SeqNum);
	//indicator
	impose_buff(packet, (char*)&data_indc, 0, sizeof(char) - 1, packet_margin);
	packet_margin += sizeof(char);
	//data
	if (data_indc) {
		impose_buff(packet, (char*)data, 0, SG_BLOCK_SIZE - 1, packet_margin);
		packet_margin += SG_BLOCK_SIZE;
	}
	//magic
	impose_buff(packet, (char*)&magic_num, 0, sizeof(uint32_t) - 1, packet_margin);
	
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
	uint32_t packet_margin = 0;
	//magic value
	uint32_t magic_v1;
	strcpy((char*)&magic_v1, packet);
	packet_margin += sizeof(uint32_t);
	//locNodeId
	impose_buff((char*)loc, packet, packet_margin, packet_margin + sizeof(SG_Node_ID) - 1, 0);
	packet_margin += sizeof(SG_Node_ID);
	//remNodeId
	impose_buff((char*)rem, packet, packet_margin, packet_margin + sizeof(SG_Node_ID) - 1, 0);
	packet_margin += sizeof(SG_Node_ID);
	//blockID
	impose_buff((char*)blk, packet, packet_margin, packet_margin + sizeof(SG_Block_ID) - 1, 0);
	packet_margin += sizeof(SG_Block_ID);
	//operation
	impose_buff((char*)op, packet, packet_margin, packet_margin + sizeof(SG_System_OP) - 1, 0);
	packet_margin += sizeof(SG_System_OP);
	//sendSeqNo
	impose_buff((char*)sseq, packet, packet_margin, packet_margin + sizeof(SG_SeqNum) - 1, 0);
	packet_margin += sizeof(SG_SeqNum);
	//recvSeqNo	
	impose_buff((char*)rseq, packet, packet_margin, packet_margin + sizeof(SG_SeqNum) - 1, 0);
	packet_margin += sizeof(SG_SeqNum);
	//indicator
	char data_indc;
	impose_buff(&data_indc, packet, packet_margin, packet_margin + sizeof(char) - 1, 0);
	packet_margin += sizeof(char);
	//data
	if (data_indc) {
		impose_buff(data, packet, packet_margin, packet_margin + SG_BLOCK_SIZE - 1, 0);
		packet_margin += SG_BLOCK_SIZE;
	} else
		data = NULL;
	uint32_t magic_v2;
	impose_buff((char*)&magic_v2, packet, packet_margin, packet_margin + sizeof(uint32_t) - 1, 0);
	packet_margin += sizeof(uint32_t);
	
	SG_Packet_Status pack_status;
	pack_status = SG_PACKT_OK;
	if (!*loc) {
		pack_status = SG_PACKT_LOCID_BAD;
		return(pack_status);
	}
	//
	if (!*rem) {
		pack_status = SG_PACKT_REMID_BAD;
		return(pack_status);
	}
	//
	if (!*blk) {
		pack_status = SG_PACKT_BLKID_BAD;
		return(pack_status);
	}
	//
	if (!(*op >= SG_INIT_ENDPOINT && *op <= SG_MAXVAL_OP)) {
		pack_status = SG_PACKT_OPERN_BAD;
		return(pack_status);
	}
	//
	if (!*sseq) {
		pack_status = SG_PACKT_SNDSQ_BAD;
		return(pack_status);
	}
	//
	if (!*rseq) {
		pack_status = SG_PACKT_RCVSQ_BAD;
		return(pack_status);
	}
	if (data_indc) {
		if (!data) {
			pack_status = SG_PACKT_BLKDT_BAD;
			return(pack_status);
		}
	}
	if (data_indc) {
		if (packet_margin != SG_DATA_PACKET_SIZE) {
			pack_status = SG_PACKT_BLKLN_BAD;
			return(pack_status);
		}
	} else
		if (packet_margin != SG_BASE_PACKET_SIZE) {
			pack_status = SG_PACKT_BLKLN_BAD;
			return(pack_status);
	}
	if (magic_v1 != SG_MAGIC_VALUE || magic_v2 != SG_MAGIC_VALUE) {
		pack_status = SG_PACKT_PDATA_BAD;
		return(pack_status);
	}

	return(pack_status);
}



















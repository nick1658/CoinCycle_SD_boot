#include "S3C2416.h"
#include "tftpserver.h"
#include "tftputils.h" 

#define MFS_MODE_READ 0
#define MFS_MODE_WRITE 1

#define TFTP_OPCODE_LEN         2
#define TFTP_BLKNUM_LEN         2
#define TFTP_ERRCODE_LEN        2
#define TFTP_DATA_LEN_MAX       512
#define TFTP_DATA_PKT_HDR_LEN   (TFTP_OPCODE_LEN + TFTP_BLKNUM_LEN)
#define TFTP_ERR_PKT_HDR_LEN    (TFTP_OPCODE_LEN + TFTP_ERRCODE_LEN)
#define TFTP_ACK_PKT_LEN        (TFTP_OPCODE_LEN + TFTP_BLKNUM_LEN)
#define TFTP_DATA_PKT_LEN_MAX   (TFTP_DATA_PKT_HDR_LEN + TFTP_DATA_LEN_MAX)
#define TFTP_MAX_RETRIES        3
#define TFTP_TIMEOUT_INTERVAL   5

typedef struct
{
  int op;    /* RRQ/WRQ */

  /* last block read */
  char data[TFTP_DATA_PKT_LEN_MAX];
  int  data_len;

  /* destination ip:port */
  struct ip_addr to_ip;
  int to_port;

  /* next block number */
  int block;

  /* total number of bytes transferred */
  int tot_bytes;

  /* timer interrupt count when last packet was sent */
  /* this should be used to resend packets on timeout */
  unsigned long long last_time;

}tftp_connection_args;

struct udp_pcb *Timeoutupcb; //超时PCB变量
struct ip_addr Timeoutaddr;
u16_t           Timeoutport;
struct udp_pcb *UDPpcb;
/* tftp_errorcode error strings */
char *tftp_errorcode_string[] = {
                                  "TimeOut!",
                                  "file not found -_-",
                                  "access violation",
                                  "disk full",
                                  "illegal operation",
                                  "unknown transfer id",
                                  "file already exists",
                                  "no such user",
                                };

void recv_callback_tftp(void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf, struct ip_addr *addr, u16_t port);

err_t tftp_send_message(struct udp_pcb *upcb, struct ip_addr *to_ip, int to_port, char *buf, int buflen)
{

  err_t err;
  struct pbuf *pkt_buf; /* Chain of pbuf's to be sent */

  /* PBUF_TRANSPORT - specifies the transport layer */
  pkt_buf = pbuf_alloc(PBUF_TRANSPORT, buflen, PBUF_POOL);

  if (!pkt_buf)      /*if the packet pbuf == NULL exit and EndTransfertransmission */
    return ERR_MEM;

  /* Copy the original data buffer over to the packet buffer's payload */
  memcpy(pkt_buf->payload, buf, buflen);

  /* Sending packet by UDP protocol */
  err = udp_sendto(upcb, pkt_buf, to_ip, to_port);

  /* free the buffer pbuf */
  pbuf_free(pkt_buf);

  return err;
}


/* construct an error message into buf using err as the error code */
int tftp_construct_error_message(char *buf, tftp_errorcode err)
{

  int errorlen;
  /* Set the opcode in the 2 first bytes */
  tftp_set_opcode(buf, TFTP_ERROR);
  /* Set the errorcode in the 2 second bytes  */
  tftp_set_errorcode(buf, err);
  /* Set the error message in the last bytes */
  tftp_set_errormsg(buf, tftp_errorcode_string[err]);
  /* Set the length of the error message  */
  errorlen = strlen(tftp_errorcode_string[err]);

  /* return message size */
  return 4 + errorlen + 1;
}

/* construct and send an error message back to client */
int tftp_send_error_message(struct udp_pcb *upcb, struct ip_addr *to, int to_port, tftp_errorcode err)
{
  char buf[512];
  int error_len;

  /* construct error */
  error_len = tftp_construct_error_message(buf, err);
  /* sEndTransfererror  */
  return tftp_send_message(upcb, to, to_port, buf, error_len);
}

/* construct and send a data packet */
int tftp_send_data_packet(struct udp_pcb *upcb, struct ip_addr *to, int to_port, int block,
                          char *buf, int buflen)
{
  char packet[TFTP_DATA_PKT_LEN_MAX]; /* (512+4) bytes */

  /* Set the opcode 3 in the 2 first bytes */
  tftp_set_opcode(packet, TFTP_DATA);
  /* Set the block numero in the 2 second bytes */
  tftp_set_block(packet, block);
  /* Set the data message in the n last bytes */
  tftp_set_data_message(packet, buf, buflen);
  /* SEndTransferthe DATA packet */
  return tftp_send_message(upcb, to, to_port, packet, buflen + 4);
}

int tftp_send_ack_packet(struct udp_pcb *upcb, struct ip_addr *to, int to_port, int block)
{

  /* create the maximum possible size packet that a TFTP ACK packet can be */
  char packet[TFTP_ACK_PKT_LEN];

  /* define the first two bytes of the packet */
  tftp_set_opcode(packet, TFTP_ACK);

  /* Specify the block number being ACK'd.
   * If we are ACK'ing a DATA pkt then the block number echoes that of the DATA pkt being ACK'd (duh)
   * If we are ACK'ing a WRQ pkt then the block number is always 0
   * RRQ packets are never sent ACK pkts by the server, instead the server sends DATA pkts to the
   * host which are, obviously, used as the "acknowledgement".  This saves from having to sEndTransferboth
   * an ACK packet and a DATA packet for RRQs - see RFC1350 for more info.  */
  tftp_set_block(packet, block);

  return tftp_send_message(upcb, to, to_port, packet, TFTP_ACK_PKT_LEN);
}



u32_t tftptimeoutEnable=0;//TFTP超时定时器
u32_t tftptimeout=0;//TFTP超时定时器
u32_t tftptimeoutNum=0;//TFTP超时定时器
/* close the file sent, disconnect and close the connection */
void tftp_cleanup_rd(struct udp_pcb *upcb, tftp_connection_args *args)
{
  /* close the filesystem */
//  file_fclose(&file_SD);
//  fs_umount(&efs1.myFs);
  /* Free the tftp_connection_args structure reserverd for */
  mem_free(args);

  /* Disconnect the udp_pcb*/
  udp_disconnect(upcb);

  /* close the connection */
  udp_remove(upcb);

  udp_recv(UDPpcb, recv_callback_tftp, NULL);
    tftptimeoutEnable=0;//关闭TFTP超时定时器
	tftptimeout=0;
}

/* close the file writen, disconnect and close the connection */
void tftp_cleanup_wr(struct udp_pcb *upcb, tftp_connection_args *args)
{
    /* close the filesystem */
//  file_fclose(&file_CR);
//  fs_umount(&efs2.myFs);
    /* Free the tftp_connection_args structure reserverd for */
	
	cy_print ("\nReceive Finished!\n");
    mem_free(args);

    /* Disconnect the udp_pcb*/
    udp_disconnect(upcb);

    /* close the connection */
    udp_remove(upcb);

    /* reset the callback function */
    udp_recv(UDPpcb, recv_callback_tftp, NULL);
    tftptimeoutEnable=0;//关闭TFTP超时定时器
	tftptimeout=0;
}
void tftp_send_next_block(struct udp_pcb *upcb, tftp_connection_args *args,
                          struct ip_addr *to_ip, u16_t to_port)
{
  /* Function to read 512 bytes from the file to sEndTransfer(file_SD), put them
   * in "args->data" and return the number of bytes read */
//  args->data_len = file_read(&file_SD, TFTP_DATA_LEN_MAX, (euint8*)args->data);
	memset (args->data, 'A', 500);
	args->data_len = 500;

  /*   NOTE: We need to sEndTransferanother data packet even if args->data_len = 0
     The reason for this is as follows:
     1) This function is only ever called if the previous packet payload was
        512 bytes.
     2) If args->data_len = 0 then that means the file being sent is an exact
         multiple of 512 bytes.
     3) RFC1350 specifically states that only a payload of <= 511 can EndTransfera
        transfer.
     4) Therefore, we must sEndTransferanother data message of length 0 to complete
        the transfer.                */


  /* sEndTransferthe data */
  tftp_send_data_packet(upcb, to_ip, to_port, args->block, args->data, args->data_len);
}

void rrq_recv_callback(void *_args, struct udp_pcb *upcb, struct pbuf *pkt_buf,
                       struct ip_addr *addr, u16_t port)
{
	
  tftp_opcode op = tftp_decode_op(pkt_buf->payload);
  /* Get our connection state  */
  tftp_connection_args *args = (tftp_connection_args *)_args;


	tftptimeout=0;//超时计时器接收到ACK后自动清空。

  if (tftp_is_correct_ack(pkt_buf->payload, args->block))
  {
    /* increment block # */
    args->block++;
  }
  else
  {
    /* we did not receive the expected ACK, so
       do not update block #. This causes the current block to be resent. */
  }

  /* if the last read returned less than the requested number of bytes
   * (i.e. TFTP_DATA_LEN_MAX), then we've sent the whole file and we can quit
   */
  if (args->data_len < TFTP_DATA_LEN_MAX)
  {
    tftptimeoutEnable=0;//关闭TFTP超时定时器
	tftptimeout=0;
    /* Clean the connection*/
    tftp_cleanup_rd(upcb, args);

    pbuf_free(pkt_buf);
  }else{
	  /* if the whole file has not yet been sent then continue  */
	  tftp_send_next_block(upcb, args, addr, port);
	  pbuf_free(pkt_buf);
  }

}
  tftp_connection_args *Timeoutargs = NULL;
int tftp_process_read(struct udp_pcb *upcb, struct ip_addr *to, int to_port, char* FileName)
{
  tftp_connection_args *args = NULL;

  /* If Could not open the file which will be transmitted  */
//  if (file_fopen(&file_SD, &efs1.myFs, FileName, 'r') != 0)
//  {
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_FILE_NOT_FOUND);

    tftp_cleanup_rd(upcb, args);

    return 0;
//  }

  /* This function is called from a callback,
   * therefore, interrupts are disabled,
   * therefore, we can use regular malloc. */

  args = mem_malloc(sizeof *args);
  /* If we aren't able to allocate memory for a "tftp_connection_args" */
  if (!args)
  {
    /* unable to allocate memory for tftp args  */
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_NOTDEFINED);

    /* no need to use tftp_cleanup_rd because no "tftp_connection_args" struct has been malloc'd   */
    tftp_cleanup_rd(upcb, args);

    return 0;
  }

   Timeoutargs=	args;//记录下当前的args
  /* initialize connection structure  */
  args->op = TFTP_RRQ;
  args->to_ip.addr = to->addr;
  args->to_port = to_port;
  args->block = 1; /* block number starts at 1 (not 0) according to RFC1350  */
  args->tot_bytes = 0;


  /* set callback for receives on this UDP PCB (Protocol Control Block) */
  udp_recv(upcb, rrq_recv_callback, args);
  Timeoutupcb =upcb;//记录下当前建立用于传输的udp控制块准备超时释放用
  
  /* initiate the transaction by sending the first block of data
   * further blocks will be sent when ACKs are received
   *   - the receive callbacks need to get the proper state    */
  tftptimeoutEnable=1;//开启TFTP超时定时器
  tftp_send_next_block(upcb, args, to, to_port);

  return 1;
}

void wrq_recv_callback(void *_args, struct udp_pcb *upcb, struct pbuf *pkt_buf, struct ip_addr *addr, u16_t port)
{
  tftp_connection_args *args = (tftp_connection_args *)_args;
//  int n = 0;

	tftptimeout=0;//超时计时器接收到ACK后自动清空。
  if (pkt_buf->len != pkt_buf->tot_len)
  {
    return;
  }

  /* Does this packet have any valid data to write? */
  if ((pkt_buf->len > TFTP_DATA_PKT_HDR_LEN) &&
      (tftp_extract_block(pkt_buf->payload) == (args->block + 1)))
  {
    /* write the received data to the file */
//    n = file_write(&file_CR,
//                   pkt_buf->len - TFTP_DATA_PKT_HDR_LEN,
//                   (euint8*)pkt_buf->payload + TFTP_DATA_PKT_HDR_LEN);
		if ((args->tot_bytes) >= sizeof (iap_code_buf))
		{
		  tftp_send_error_message(upcb, addr, port, TFTP_ERR_DISKFULL);
		  /* close the connection */
		  tftp_cleanup_wr(upcb, args); /* close the connection */
		}else{
			memcpy (&iap_code_buf[args->tot_bytes], (uint8*)pkt_buf->payload + TFTP_DATA_PKT_HDR_LEN,
						pkt_buf->len - TFTP_DATA_PKT_HDR_LEN);
		}

    /* update our block number to match the block number just received */
    args->block++;
    /* update total bytes  */
    (args->tot_bytes) += (pkt_buf->len - TFTP_DATA_PKT_HDR_LEN);

    /* This is a valid pkt but it has no data.  This would occur if the file being
       written is an exact multiple of 512 bytes.  In this case, the args->block
       value must still be updated, but we can skip everything else.    */
  }
  else if (tftp_extract_block(pkt_buf->payload) == (args->block + 1))
  {
    /* update our block number to match the block number just received  */
    args->block++;
  }

  /* SEndTransferthe appropriate ACK pkt (the block number sent in the ACK pkt echoes
   * the block number of the DATA pkt we just received - see RFC1350)
   * NOTE!: If the DATA pkt we received did not have the appropriate block
   * number, then the args->block (our block number) is never updated and
   * we simply sEndTransfera "duplicate ACK" which has the same block number as the
   * last ACK pkt we sent.  This lets the host know that we are still waiting
   * on block number args->block+1. */
  tftp_send_ack_packet(upcb, addr, port, args->block);

  /* If the last write returned less than the maximum TFTP data pkt length,
   * then we've received the whole file and so we can quit (this is how TFTP
   * signals the EndTransferof a transfer!)
   */
  if (pkt_buf->len < TFTP_DATA_PKT_LEN_MAX)
  {
    tftp_cleanup_wr(upcb, args);
    pbuf_free(pkt_buf);
    sys_env.tty_online_ms = 2;//通知更新完成
	sys_env.update_flag = NET_UPDATE;
  }
  else
  {
    pbuf_free(pkt_buf);
	if (args->block % (64) == 0){
		cy_print ("\n");
	}
	cy_print (".");
    return;
  }

}



int tftp_process_write(struct udp_pcb *upcb, struct ip_addr *to, int to_port, s_tftp_cmd *p_tftp_cmd)
{
  tftp_connection_args *args = NULL;

  /* If Could not open the file which will be transmitted  */
//  if (file_fopen(&file_CR, &efs2.myFs, FileName, 'w') != 0)
//  {
//    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_FILE_ALREADY_EXISTS);

//    tftp_cleanup_wr(upcb, args);

//    return 0;
//  }
																			
  /* This function is called from a callback,
   * therefore interrupts are disabled,
   * therefore we can use regular malloc   */
  args = mem_malloc(sizeof *args);
  if (!args)
  {
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_NOTDEFINED);

    tftp_cleanup_wr(upcb, args);

    return 0;
  }

   Timeoutargs=	args;//记录下当前的args
	args->op = TFTP_WRQ;
	args->to_ip.addr = to->addr;
	args->to_port = to_port;
	/* the block # used as a positive response to a WRQ is _always_ 0!!! (see RFC1350)  */
	args->block = 0;
	args->tot_bytes = 0;

	if ((strcmp (p_tftp_cmd->op, "command") == 0) && (strcmp (p_tftp_cmd->op_v, "reset") == 0)){
		tftp_send_ack_packet(upcb, to, to_port, args->block);
		tftp_cleanup_wr(upcb, args);
		cy_print ("Reset...");
		sys_env.net_task = 50;
	}else{	
		/* set callback for receives on this UDP PCB (Protocol Control Block) */
		udp_recv(upcb, wrq_recv_callback, args);
		Timeoutupcb =upcb;//记录下当前建立用于传输的udp控制块准备超时释放用

		/* initiate the write transaction by sending the first ack */
		tftp_send_ack_packet(upcb, to, to_port, args->block);
		cy_print (".");
		tftptimeoutEnable=1;//开启TFTP超时定时器
	}

	return 0;
}


/* for each new request (data in p->payload) from addr:port,
 * create a new port to serve the response, and start the response
 * process
 *///
void process_tftp_request(struct pbuf *pkt_buf, struct ip_addr *addr, u16_t port)
{
  tftp_opcode op = tftp_decode_op(pkt_buf->payload);
  //char FileName[30];
	s_tftp_cmd tftp_cmd;
  struct udp_pcb *upcb;
  err_t err;
	
//解除注册，使TFTP不接受其他在69上的文件传输请求
 udp_recv(UDPpcb,  NULL, NULL);
  

  /* create new UDP PCB structure */
  upcb = udp_new();
  if (!upcb)
  { 
    udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调   
   /* Error creating PCB. Out of Memory  */
    return;
  }
  /* bind to port 0 to receive next available free port */
  /* NOTE:  This is how TFTP works.  There is a UDP PCB for the standard port
   * 69 which al transactions begin communication on, however, _all_ subsequent
   * transactions for a given "stream" occur on another port!  */
  err = udp_bind(upcb, IP_ADDR_ANY, 8888);
  if (err != ERR_OK)
  {   
  	 udp_remove(upcb);
   /* Unable to bind to port   */
   udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调  
    return;
  }


	sys_env.update_flag = NET_UPDATEING;
  switch (op)
  {

    case TFTP_RRQ:    /* TFTP RRQ (read request)  */
      /* Read the name of the file asked by the client to be sent from the SD card */
      tftp_extract_filename(&tftp_cmd, pkt_buf->payload, pkt_buf->len);

      /* If Could not open filesystem */
//      if (efs_init(&efs1, 0) != 0)
//      {
//	    /* sEndTransfera generic access violation message */
//      tftp_send_error_message(upcb, addr, port, TFTP_ERR_FILE_NOT_FOUND);
//      /* TFTP unknown request op */
//      /* no need to use tftp_cleanup_wr because no "tftp_connection_args" struct has been malloc'd   */
//      udp_remove(upcb);
//	  udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调    
//        return;
//      }
//      /* If Could not open the selected directory */
//      if (ls_openDir(&list1, &(efs1.myFs), "/") != 0)
//		{
//			/* sEndTransfera generic access violation message */
//			tftp_send_error_message(upcb, addr, port, TFTP_ERR_FILE_NOT_FOUND);
//			/* TFTP unknown request op */
//			/* no need to use tftp_cleanup_wr because no "tftp_connection_args" struct has been malloc'd   */
//			udp_remove(upcb);
//			udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调   
//			return;
//      }
	  Timeoutaddr=*addr;
	  Timeoutport=port;//记录下来
	  //Timeoutportpkt_buf=pkt_buf;
      /* Start the TFTP read mode*/
      tftp_process_read(upcb, addr, port, tftp_cmd.file_name);
      break;

    case TFTP_WRQ:    /* TFTP WRQ (write request)   */
		/* Read the name of the file asked by the client to received and writen in the SD card */
		tftp_extract_filename(&tftp_cmd, pkt_buf->payload, pkt_buf->len);
		cy_print ("Receive File Name: %s\n", tftp_cmd.file_name);
      /* If Could not open filesystem */
//      if (efs_init(&efs2, 0) != 0)
//      {
//	      /* sEndTransfera generic access violation message */
//      tftp_send_error_message(upcb, addr, port, TFTP_ERR_FILE_NOT_FOUND);
//      /* TFTP unknown request op */
//      /* no need to use tftp_cleanup_wr because no "tftp_connection_args" struct has been malloc'd   */
//      udp_remove(upcb);
//	  udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调    
//        return;
//      }
//      /* If Could not open the selected directory */
//      if (ls_openDir(&list2, &(efs2.myFs), "/") != 0)
//      {
//	     /* sEndTransfera generic access violation message */
//      tftp_send_error_message(upcb, addr, port, TFTP_ERR_FILE_NOT_FOUND);
//      /* TFTP unknown request op */
//      /* no need to use tftp_cleanup_wr because no "tftp_connection_args" struct has been malloc'd   */
//      udp_remove(upcb);
//	  udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调   
//        return;
//      }

      /* Start the TFTP write mode*/
		Timeoutaddr=*addr;
		Timeoutport=port;//记录下来
		tftp_process_write(upcb, addr, port, &tftp_cmd);
      break;

    default:

		sys_env.update_flag = NULL_UPDATE;
		/* sEndTransfera generic access violation message */
		tftp_send_error_message(upcb, addr, port, TFTP_ERR_ACCESS_VIOLATION);
		/* TFTP unknown request op */
		/* no need to use tftp_cleanup_wr because no "tftp_connection_args" struct has been malloc'd   */
		udp_remove(upcb);
		udp_recv(UDPpcb, recv_callback_tftp, NULL);//重新注册69端口回调  
      break;
  }

}	

void recv_callback_tftp (void *arg, struct udp_pcb *upcb, struct pbuf *pkt_buf, ip_addr_t *addr, u16_t port)
{
	/* process new connection request */
	process_tftp_request(pkt_buf, addr, port);
	pbuf_free(pkt_buf); 
}




void tftp_init(void)
{ 
  err_t err;
  unsigned port = 69;

  /* create a new UDP PCB structure  */
  UDPpcb = udp_new();
  if (!UDPpcb)
  {  /* Error creating PCB. Out of Memory  */
    return;
  }

  /* Bind this PCB to port 69  */
  err = udp_bind(UDPpcb, IP_ADDR_ANY, port);
  if (err != ERR_OK)
  {    /* Unable to bind to port  */
    return;
  }

  /* TFTP server start  */
  udp_recv(UDPpcb, recv_callback_tftp, NULL);
}

void TftpTrm250ms(void) //250MS调用一次
{

	if( tftptimeoutEnable){//如果开启超时定时器则进入
	
		tftptimeout++;
		if(tftptimeout > 13){ //超过3S之后结束UDPPCB释放BUF。
			tftp_send_error_message(Timeoutupcb, &Timeoutaddr, Timeoutport, TFTP_ERR_NOTDEFINED);
			tftp_cleanup_rd(Timeoutupcb, Timeoutargs);	
			dbg ("TFTP Server TimeOut\n");
			sys_env.update_flag = NULL_UPDATE;
		}else if ((tftptimeout % 2) == 0){
			tftp_send_ack_packet(Timeoutupcb, &Timeoutaddr, Timeoutport, Timeoutargs->block);
		}
	}
}


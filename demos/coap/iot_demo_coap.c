/*
 * /*
 *  Created on: Nov 10, 2020
 *  Authors: Mohammed Abdelmaksoud & Hatim Jamali
 *  1NCE GmbH
 *
 */

/**
 * @file iot_demo_coap.c
 * @brief Demonstrates usage of lobaro COAP library.
 */
#include "nce_demo_config.h"
#if defined(CONFIG_COAP_DEMO_ENABLED)
/* The config header is always included first. */
#include "iot_config.h"

/* Standard includes. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Set up logging for this demo. */
#include "iot_demo_logging.h"

/* Platform layer includes. */
#include "platform/iot_clock.h"
#include "platform/iot_threads.h"
#include "platform/iot_network.h"

/* COAP include. */
#include "coap_main.h"
#include "transport_secure_sockets.h"
#include "udp_impl.h"


/*-----------------------------------------------------------*/



CoAP_RespHandler_fn_t CoAP_Resp_handler( CoAP_Message_t * pRespMsg,
                                         NetEp_t * Sender )
{
    IotLogInfo( "MESSAGE Payload : %s \r\n", pRespMsg->Payload );
    PrintEndpoint( Sender );
}

/* Declaration of demo function. */
int RuncoapDemo( bool awsIotMode,
                 const char * pIdentifier,
                 void * pNetworkServerInfo,
                 void * pNetworkCredentialInfo,
                 const IotNetworkInterface_t * pNetworkInterface );

/*-----------------------------------------------------------*/

/**
 * @brief The function that runs the COAP demo, called by the demo runner.
 * @return `EXIT_SUCCESS` if the demo completes successfully; `EXIT_FAILURE` otherwise.
 */
int RuncoapDemo( bool awsIotMode,
                 const char * pIdentifier,
                 void * pNetworkServerInfo,
                 void * pNetworkCredentialInfo,
                 const IotNetworkInterface_t * pNetworkInterface )
{
    /* Return value of this function and the exit status of this program. */
    int status = EXIT_FAILURE;
    SocketsSockaddr_t ServerAddress;
    NetPacket_t pPacket;
    NetworkContext_t xNetworkContext = { 0 };
    IotLogInfo( "Connecting to CoAP server\r\n" );
    ServerAddress.usPort = SOCKETS_htons( configCOAP_PORT );
    ServerAddress.ulAddress = SOCKETS_GetHostByName( COAP_ENDPOINT );

    /* Check for errors from DNS lookup. */
    if( ServerAddress.ulAddress == ( uint32_t ) 0 )
    {
        IotLogError( "Failed to connect to server: DNS resolution failed: Server=%s.",
                     COAP_ENDPOINT );
    }
    else
    {
        uint8_t configCOAP_SERVER_ADDR3 = ( uint8_t ) ( ServerAddress.ulAddress >> 24 );
        uint8_t configCOAP_SERVER_ADDR2 = ( uint8_t ) ( ServerAddress.ulAddress >> 16 );
        uint8_t configCOAP_SERVER_ADDR1 = ( uint8_t ) ( ServerAddress.ulAddress >> 8 );
        uint8_t configCOAP_SERVER_ADDR0 = ( uint8_t ) ( ServerAddress.ulAddress >> 0 );
        const NetEp_t ServerEp =
        {
            .NetType = IPV4, .NetPort =
                configCOAP_PORT, .NetAddr= { .IPv4 = { .u8 =
                                   {
                                       configCOAP_SERVER_ADDR0, configCOAP_SERVER_ADDR1,
                                       configCOAP_SERVER_ADDR2, configCOAP_SERVER_ADDR3
                                   } } }
        };


        /* Create UDP Socket */
        Socket_t udp = SOCKETS_Socket( SOCKETS_AF_INET, SOCKETS_SOCK_DGRAM,
                                       SOCKETS_IPPROTO_UDP );
        CoAP_Socket_t * socket = AllocSocket();
        #ifdef ENABLE_DTLS
            SOCKETS_SetSockOpt( udp, 0, SOCKETS_SO_REQUIRE_TLS, NULL, ( size_t ) 0 );
        #endif

        /* Connect to CoAP server */
        if( SOCKETS_Connect( udp, &ServerAddress, sizeof( ServerAddress ) ) == 0 )
        {
            IotLogInfo( "Connected to CoAP server \r\n" );
            #ifdef TROUBLESHOOTING
                char pcTransmittedString[ 160 ];

                /* Add payload */
                nce_troubleshooting( &pcTransmittedString );
            #else
				#ifndef CONFIG_NCE_ENERGY_SAVER
                	/* Add payload */
                	char pcTransmittedString[]=PUBLISH_PAYLOAD_FORMAT;
				#else
                /* Add payload */
                char pcTransmittedString[500];
                (void) memset(&pcTransmittedString, (uint8_t) '\0', sizeof(pcTransmittedString));
                uint8_t selector = 4;

            	Element2byte_gen_t battery_level = {.type= E_INTEGER,.value.i=99,.template_length=1};
                Element2byte_gen_t signal_strength = {.type= E_INTEGER,.value.i=84,.template_length=1};
                Element2byte_gen_t software_version = {.type= E_STRING,.value.s="2.2.1",.template_length=5};
                os_energy_save(pcTransmittedString,selector, 3,battery_level,signal_strength,software_version);

                #endif
			#endif
            /* Add socket and Network Interface configuration */
            socket->Handle = udp;
            socket->Tx = CoAP_Send_Wifi;

            /* Create confirmable CoAP POST packet with URI Path option */
            CoAP_Message_t * pReqMsg = CoAP_CreateMessage( CON, REQ_POST,
                                                           CoAP_GetNextMid(), CoAP_GenerateToken() );

            CoAP_addNewPayloadToMessage( pReqMsg, pcTransmittedString,
                                         strlen( pcTransmittedString ) );
            CoAP_AddOption( pReqMsg, OPT_NUM_URI_QUERY, democonfigCLIENT_IDENTIFIER, strlen( democonfigCLIENT_IDENTIFIER ) );


            /* Create CoAP Client Interaction to send a confirmable POST Request  */
            CoAP_StartNewClientInteraction( pReqMsg, socket->Handle, &ServerEp, CoAP_Resp_handler );
            CoAP_Interaction_t * pIA = CoAP_GetLongestPendingInteraction();


            /* Execute the Interaction list  */
            CoAP_Message_t * pMsg = NULL;

            while( pIA != NULL || pIA->State == COAP_STATE_FINISHED)
            {
                CoAP_doWork();

                if( pIA->State == COAP_STATE_WAITING_RESPONSE )
                {
                    CoAP_Recv_Wifi( socket->Handle, &pPacket, ServerEp );
                }
                if(pIA->next != NULL){
                	pIA = CoAP_GetLongestPendingInteraction();
                }else{
                	if(pIA->State == COAP_STATE_FINISHED){
                		pIA= pIA->next;
                		status = EXIT_SUCCESS;
                	}
                }

                if( troubleshooting.Code != RESP_SUCCESS_CHANGED_2_04 )
                {
                    #ifdef TROUBLESHOOTING
                        if( nce_send_sms( pcTransmittedString ) == 0 )
                        {
                            IotLogInfo( "Failed to send message Via COAP The Message Sent via SMS \r\n" );
                            status = EXIT_SUCCESS;
                        }
                        else
                        {
                            IotLogInfo( "Failed to Send the Message either via COAP or SMS \r\n" );
                            status = EXIT_FAILURE;
                        }
                    #else  /* ifdef TROUBLESHOOTING */
                        IotLogInfo( "Failed to Send the Message \r\n" );
                        status = EXIT_FAILURE;
                    #endif /* ifdef TROUBLESHOOTING */
                }
            }
        }
    }

    return status;
}

/*-----------------------------------------------------------*/
#endif

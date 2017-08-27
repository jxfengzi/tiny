/**
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   StreamServerChannel.c
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 */

#include <tiny_malloc.h>
#include <tiny_log.h>
#include <tiny_inet.h>
#include <tiny_socket.h>
#include "StreamServerChannel.h"
#include "StreamServerChannelContext.h"
#include "channel/SocketChannel.h"

#define TAG     "StreamServerChannel"

TINY_LOR
static void StreamServerChannel_Dispose(Channel *thiz)
{
    if (thiz->ctx != NULL)
    {
        StreamServerChannelContext_Delete((StreamServerChannelContext *)thiz->ctx);
        thiz->ctx = NULL;
    }

    SocketChannel_Dispose(thiz);
}

TINY_LOR
static void StreamServerChannel_Delete(Channel *thiz)
{
    StreamServerChannel_Dispose(thiz);
    tiny_free(thiz);
}

TINY_LOR
static void StreamServerChannel_OnRegister(Channel *thiz, Selector *selector, ChannelTimer *timer)
{
    if (Channel_IsActive(thiz))
    {
        StreamServerChannelContext *ctx = (StreamServerChannelContext *)thiz->ctx;
        Selector_Register(selector, thiz->fd, SELECTOR_OP_READ);
        LOG_D(TAG, "StreamServerChannel_OnRegister Server FD: %d", thiz->fd);

        // remove closed channel
        for (int i = (ctx->channels.size - 1); i >= 0; --i)
        {
            Channel* child = (Channel *)TinyList_GetAt(&ctx->channels, i);
            if (Channel_IsClosed(child))
            {
                LOG_D(TAG, "remove channel[%d]: %s, fd: %d", i, child->id, child->fd);
                TinyList_RemoveAt(&ctx->channels, i);
            }
        }

        for (uint32_t i = 0; i < ctx->channels.size; ++i)
        {
            Channel *child = (Channel *) TinyList_GetAt(&ctx->channels, i);
            Selector_Register(selector, child->fd, SELECTOR_OP_READ);

            LOG_D(TAG, "StreamServerChannel_OnRegister Connnection FD: %d", child->fd);

            if (child->getTimeout != NULL)
            {
                if (RET_SUCCEEDED(child->getTimeout(child, timer, NULL)))
                {
                    timer->fd = child->fd;
                }
            }
        }
    }
}

TINY_LOR
static void StreamServerChannel_OnRemove(Channel *thiz)
{
    LOG_D(TAG, "StreamServerChannel_OnRemove");

    StreamServerChannel_Delete(thiz);
}

TINY_LOR
static TinyRet StreamServerChannel_OnReadWrite(Channel *thiz, Selector *selector)
{
    StreamServerChannelContext *ctx = (StreamServerChannelContext *)thiz->ctx;

    if (Selector_IsReadable(selector, thiz->fd))
    {
        Channel *newChannel = NULL;
        int fd = 0;
        struct sockaddr_in addr;
        socklen_t len = (socklen_t) sizeof(addr);
        char ip[TINY_IP_LEN];
        uint16_t port = 0;

        LOG_D(TAG, "StreamServerChannel_OnRead FD: %d", thiz->fd);
        LOG_D(TAG, "socklen_t: %d", len);

        memset(&addr, 0, sizeof(addr));
        fd = tiny_accept(thiz->fd, (struct sockaddr *)&addr, &len);
        if (fd < 0)
        {
            return TINY_RET_E_INTERNAL;
        }

        memset(ip, 0, TINY_IP_LEN);
        inet_ntop(AF_INET, &addr.sin_addr, ip, TINY_IP_LEN);
        port = ntohs(addr.sin_port);

        LOG_D(TAG, "accept a new connection: %s:%d, FD: %d", ip, port, fd);

        newChannel = SocketChannel_New();
        if (newChannel == NULL)
        {
            printf("SocketChannel_New NULL\n");
            return TINY_RET_OK;
        }

        newChannel->fd = fd;
        SocketChannel_SetRemoteInfo(newChannel, ip, port);
        SocketChannel_Initialize(newChannel, ctx->initializer, ctx->initializerContext);

        TinyList_AddTail(&ctx->channels, newChannel);

        return TINY_RET_OK;
    }

    for (uint32_t i = 0; i < ctx->channels.size; ++i)
    {
        Channel *channel = (Channel *) TinyList_GetAt(&ctx->channels, i);
        if (Channel_IsActive(channel))
        {
            if (RET_FAILED(channel->onReadWrite(channel, selector)))
            {
                LOG_D(TAG, "close connection: %s:%d, FD:%d", channel->local.socket.ip, channel->local.socket.port, channel->fd);
                channel->onInactive(channel);
                Channel_Close(channel);
            }
        }
    }

    for (uint32_t i = 0; i < ctx->channels.size; ++i)
    {
        Channel *channel = (Channel *)TinyList_GetAt(&ctx->channels, i);
        if (Channel_IsClosed(channel))
        {
            TinyList_RemoveAt(&ctx->channels, i);
            break;
        }
    }

    return TINY_RET_OK;
}

TINY_LOR
static void StreamServerChannel_OnInactive(Channel *thiz)
{
    StreamServerChannelContext *ctx = (StreamServerChannelContext *)thiz->ctx;

    LOG_D(TAG, "StreamServerChannel_OnInactive");

    for (uint32_t i = 0; i < ctx->channels.size; ++i)
    {
        Channel *channel = (Channel *)TinyList_GetAt(&ctx->channels, i);
        channel->onInactive(channel);
        Channel_Close(channel);
    }
}

TINY_LOR
static void StreamServerChannel_OnEventTriggered(Channel *thiz, ChannelTimer *timer)
{
    StreamServerChannelContext *ctx = (StreamServerChannelContext *)thiz->ctx;

    RETURN_IF_FAIL(thiz);

    LOG_D(TAG, "StreamServerChannel_OnEventTriggered");

    for (uint32_t i = 0; i < ctx->channels.size; ++i)
    {
        Channel *channel = (Channel *)TinyList_GetAt(&ctx->channels, i);
        channel->onEventTriggered(channel, timer);
    }
}

TINY_LOR
static TinyRet StreamServerChannel_Construct(Channel *thiz, int maxConnections)
{
    TinyRet ret = TINY_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, TINY_RET_E_ARG_NULL);

    LOG_D(TAG, "StreamServerChannel_Construct");

    do
    {
        thiz->onRegister = StreamServerChannel_OnRegister;
        thiz->onRemove = StreamServerChannel_OnRemove;
        thiz->onInactive = StreamServerChannel_OnInactive;
        thiz->onReadWrite = StreamServerChannel_OnReadWrite;
        thiz->onEventTriggered = StreamServerChannel_OnEventTriggered;

        thiz->ctx = StreamServerChannelContext_New();
        if (thiz->ctx == NULL)
        {
            ret = TINY_RET_E_OUT_OF_MEMORY;
            break;
        }

        ((StreamServerChannelContext *)thiz->ctx)->maxConnections = maxConnections;
    } while (0);

    return ret;
}

TINY_LOR
Channel * StreamServerChannel_New(int maxConnections)
{
    Channel * thiz = NULL;

    do
    {
        thiz = SocketChannel_New();
        if (thiz == NULL)
        {
            break;
        }

        if (RET_FAILED(StreamServerChannel_Construct(thiz, maxConnections)))
        {
            StreamServerChannel_Delete(thiz);
            thiz = NULL;
            break;
        }
    } while (0);

    return thiz;
}

TINY_LOR
void StreamServerChannel_Initialize(Channel *thiz, ChannelInitializer initializer, void *ctx)
{
    RETURN_IF_FAIL(thiz);
    RETURN_IF_FAIL(initializer);

    ((StreamServerChannelContext *)thiz->ctx) ->initializer = initializer ;
    ((StreamServerChannelContext *)thiz->ctx) ->initializerContext = ctx;
}
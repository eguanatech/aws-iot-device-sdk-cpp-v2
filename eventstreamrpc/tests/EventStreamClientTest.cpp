/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */
#include <aws/crt/Api.h>

#include <aws/eventstreamrpc/EventStreamClient.h>

#include <aws/testing/aws_test_harness.h>

#include <iostream>
#include <sstream>

using namespace Aws::Eventstreamrpc;

static int s_TestEventStreamConnect(struct aws_allocator *allocator, void *ctx)
{
    (void)ctx;
    {
        Aws::Crt::ApiHandle apiHandle(allocator);
        Aws::Crt::Io::TlsContextOptions tlsCtxOptions = Aws::Crt::Io::TlsContextOptions::InitDefaultClient();
        Aws::Crt::Io::TlsContext tlsContext(tlsCtxOptions, Aws::Crt::Io::TlsMode::CLIENT, allocator);
        ASSERT_TRUE(tlsContext);

        Aws::Crt::Io::TlsConnectionOptions tlsConnectionOptions = tlsContext.NewConnectionOptions();
        Aws::Crt::Io::SocketOptions socketOptions;
        socketOptions.SetConnectTimeoutMs(1000);

        Aws::Crt::Io::EventLoopGroup eventLoopGroup(0, allocator);
        ASSERT_TRUE(eventLoopGroup);

        Aws::Crt::Io::DefaultHostResolver defaultHostResolver(eventLoopGroup, 8, 30, allocator);
        ASSERT_TRUE(defaultHostResolver);

        Aws::Crt::Io::ClientBootstrap clientBootstrap(eventLoopGroup, defaultHostResolver, allocator);
        ASSERT_TRUE(clientBootstrap);
        clientBootstrap.EnableBlockingShutdown();
        MessageAmendment connectionAmendment;
        auto messageAmender = [&](void) -> MessageAmendment & { return connectionAmendment; };
        EventstreamRpcConnection *connection;
        int lastErrorCode = AWS_OP_SUCCESS;

        std::condition_variable semaphore;
        std::mutex semaphoreLock;

        auto onConnect = [&](EventstreamRpcConnection *newConnection) {
            std::lock_guard<std::mutex> lockGuard(semaphoreLock);

            connection = newConnection;

            semaphore.notify_one();
        };

        auto onDisconnect = [&](int errorCode) {
            std::lock_guard<std::mutex> lockGuard(semaphoreLock);

            lastErrorCode = errorCode;

            semaphore.notify_one();
        };

        EventstreamRpcConnectionOptions options;
        options.Bootstrap = &clientBootstrap;
        options.SocketOptions = socketOptions;
        options.HostName = Aws::Crt::String("127.0.0.1");
        options.Port = 8033;
        options.ConnectMessageAmenderCallback = std::move(messageAmender);
        options.OnConnectCallback = std::move(onConnect);
        options.OnDisconnectCallback = std::move(onDisconnect);
        options.OnErrorCallback = nullptr;
        options.OnPingCallback = nullptr;

        std::unique_lock<std::mutex> semaphoreULock(semaphoreLock);

        /* Happy path case. */
        {
            connectionAmendment.AddHeader(EventStreamHeader(
                Aws::Crt::String("client-name"), Aws::Crt::String("accepted.testy_mc_testerson"), allocator));
            ASSERT_TRUE(EventstreamRpcConnection::CreateConnection(options, allocator));
            semaphore.wait(semaphoreULock, [&]() { return connection; });
            ASSERT_NOT_NULL(connection);
            connection->Close();
            semaphore.wait(semaphoreULock, [&]() { return lastErrorCode == AWS_OP_SUCCESS; });
            ASSERT_TRUE(lastErrorCode == AWS_OP_SUCCESS);
        }

        /* Rejected client-name header. */
        {
            connectionAmendment.AddHeader(EventStreamHeader(
                Aws::Crt::String("client-name"), Aws::Crt::String("rejected.testy_mc_testerson"), allocator));
            ASSERT_TRUE(EventstreamRpcConnection::CreateConnection(options, allocator));
            semaphore.wait(
                semaphoreULock, [&]() { return lastErrorCode == AWS_ERROR_EVENT_STREAM_RPC_CONNECTION_CLOSED; });
            ASSERT_INT_EQUALS(AWS_ERROR_EVENT_STREAM_RPC_CONNECTION_CLOSED, lastErrorCode);
        }

        /* Empty amendment headers. */
        {
            ASSERT_TRUE(EventstreamRpcConnection::CreateConnection(options, allocator));
            semaphore.wait(
                semaphoreULock, [&]() { return lastErrorCode == AWS_ERROR_EVENT_STREAM_RPC_CONNECTION_CLOSED; });
            ASSERT_INT_EQUALS(AWS_ERROR_EVENT_STREAM_RPC_CONNECTION_CLOSED, lastErrorCode);
        }
    }

    return AWS_OP_SUCCESS;
}

AWS_TEST_CASE(EventStreamConnect, s_TestEventStreamConnect)

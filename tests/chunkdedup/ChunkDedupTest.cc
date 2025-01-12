/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include <gtest/gtest.h>
#include <pulsar/Client.h>

#include "lib/Latch.h"
#include "lib/LogUtils.h"

DECLARE_LOG_OBJECT()

using namespace pulsar;

// Before https://github.com/apache/pulsar/pull/20948, when message deduplication is enabled, sending chunks
// to the broker will receive send error response.
TEST(ChunkDedupTest, testSendChunks) {
    Client client{"pulsar://localhost:6650"};
    ProducerConfiguration conf;
    conf.setBatchingEnabled(false);
    conf.setChunkingEnabled(true);
    Producer producer;
    ASSERT_EQ(ResultOk, client.createProducer("test-send-chunks", conf, producer));

    Latch latch{1};
    std::string value(1024000 /* max message size */ * 100, 'a');
    producer.sendAsync(MessageBuilder().setContent(value).build(),
                       [&latch](Result result, const MessageId& msgId) {
                           LOG_INFO("Send to " << msgId << ": " << result);
                           latch.countdown();
                       });
    ASSERT_TRUE(latch.wait(std::chrono::seconds(10)));
    client.close();
}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

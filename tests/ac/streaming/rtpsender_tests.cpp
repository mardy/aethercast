/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gmock/gmock.h>

#include <boost/concept_check.hpp>

#include "ac/network/stream.h"

#include "ac/streaming/rtpsender.h"

using namespace ::testing;

namespace {
static constexpr unsigned int kRTPHeaderSize{12};
static constexpr unsigned int kStreamMaxUnitSize = 1472;
static constexpr unsigned int kMPEGTSPacketSize{188};
static constexpr unsigned int kSourceID = 0xdeadbeef;
// See http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml
static constexpr unsigned int kRTPPayloadTypeMP2T = 33;

class MockNetworkStream : public ac::network::Stream {
public:
    MOCK_METHOD2(Connect, bool(const std::string &address, const ac::network::Port &port));
    MOCK_METHOD3(Write, ac::network::Stream::Error(const uint8_t*, unsigned int, const ac::TimestampUs&));
    MOCK_CONST_METHOD0(LocalPort, ac::network::Port());
    MOCK_CONST_METHOD0(MaxUnitSize, std::uint32_t());
};

class MockSenderReport : public ac::video::SenderReport {
public:
    MOCK_METHOD2(SentPacket, void(const ac::TimestampUs&, const size_t&));
};
}

TEST(RTPSender, ForwardsCorrectPort) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    ac::network::Port local_port = 1234;

    EXPECT_CALL(*mock_stream, LocalPort())
            .Times(1)
            .WillOnce(Return(local_port));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    EXPECT_EQ(local_port, sender->LocalPort());
}

TEST(RTPSender, SpecifiesExecutableName) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);
    EXPECT_NE(0, sender->Name().length());
}

TEST(RTPSender, StartAndStopDoesNotFail) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    EXPECT_TRUE(sender->Start());
    EXPECT_TRUE(sender->Stop());
}

TEST(RTPSender, DoeNotAcceptIncorrectPacketSizes) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    auto packets = ac::video::Buffer::Create(kMPEGTSPacketSize + 1);
    EXPECT_FALSE(sender->Queue(packets));

    packets = ac::video::Buffer::Create(kMPEGTSPacketSize - 1);
    EXPECT_FALSE(sender->Queue(packets));
}

TEST(RTPSender, ExecutesWithEmptyQueue) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    EXPECT_TRUE(sender->Execute());
}

TEST(RTPSender, QueuesUpPackagesAndSendsAll) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    auto now = ac::Utils::GetNowUs();

    EXPECT_CALL(*mock_report, SentPacket(now, _))
            .Times(3);

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    EXPECT_CALL(*mock_stream, Write(_, _, _))
            .Times(3)
            .WillRepeatedly(Return(ac::network::Stream::Error::kNone));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    auto packets = ac::video::Buffer::Create(kMPEGTSPacketSize * 15);
    packets->SetTimestamp(now);

    EXPECT_TRUE(sender->Queue(packets));
    EXPECT_TRUE(sender->Execute());
}

TEST(RTPSender, WritePackageFails) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    EXPECT_CALL(*mock_stream, Write(_, _, _))
            .Times(1)
            .WillOnce(Return(ac::network::Stream::Error::kRemoteClosedConnection));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    auto packets = ac::video::Buffer::Create(kMPEGTSPacketSize);

    EXPECT_TRUE(sender->Queue(packets));
    EXPECT_FALSE(sender->Execute());
}

TEST(RTPSender, ConstructsCorrectRTPHeader) {
    auto mock_stream = std::make_shared<MockNetworkStream>();
    auto mock_report = std::make_shared<MockSenderReport>();

    auto packet_timestamp = ac::Utils::GetNowUs();

    uint32_t expected_output_size = kMPEGTSPacketSize + kRTPHeaderSize;

    EXPECT_CALL(*mock_report, SentPacket(packet_timestamp, expected_output_size))
            .Times(1);

    EXPECT_CALL(*mock_stream, MaxUnitSize())
            .WillRepeatedly(Return(kStreamMaxUnitSize));

    std::uint8_t *output_data = nullptr;

    EXPECT_CALL(*mock_stream, Write(_, expected_output_size, _))
            .WillOnce(DoAll(Invoke([&](const uint8_t *data, unsigned int size, const ac::TimestampUs &timestamp) {
                                boost::ignore_unused_variable_warning(timestamp);

                                // Need to copy the data here as otherwise its freed
                                // as soon as the write call comes back.
                                output_data = new std::uint8_t[size];
                                ::memcpy(output_data, data, size);
                            }),
                            Return(ac::network::Stream::Error::kNone)));

    auto sender = std::make_shared<ac::streaming::RTPSender>(mock_stream, mock_report);

    auto packets = ac::video::Buffer::Create(kMPEGTSPacketSize);
    packets->SetTimestamp(packet_timestamp);

    EXPECT_TRUE(sender->Queue(packets));
    EXPECT_TRUE(sender->Execute());

    EXPECT_NE(nullptr, output_data);

    EXPECT_EQ(0x80, output_data[0]);
    EXPECT_EQ(kRTPPayloadTypeMP2T, output_data[1]);

    // Sequence should start at 0
    EXPECT_EQ(0x00, output_data[2]);
    EXPECT_EQ(0x00, output_data[3]);

    // We can't compare the actual RTP time here but we can make sure
    // its between our start time and now.
    std::uint32_t rtp_time = 0;
    rtp_time |= (output_data[4] << 24);
    rtp_time |= (output_data[5] << 16);
    rtp_time |= (output_data[6] << 8);
    rtp_time |= (output_data[7] << 0);

    const uint32_t packet_timestamp_90khz = (packet_timestamp * 9) / 100ll;
    const uint32_t now_rtp_time = (ac::Utils::GetNowUs() * 9) / 100ll;

    EXPECT_LE(packet_timestamp_90khz, rtp_time);
    EXPECT_GE(now_rtp_time, rtp_time);

    std::uint32_t source_id = 0;
    source_id |= (output_data[8] << 24);
    source_id |= (output_data[9] << 16);
    source_id |= (output_data[10] << 8);
    source_id |= (output_data[11] << 0);

    EXPECT_EQ(kSourceID, source_id);

    if (output_data)
        delete output_data;
}

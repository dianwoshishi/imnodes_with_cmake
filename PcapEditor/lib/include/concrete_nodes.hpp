#pragma once
#include <sstream>
#include <imgui_extensions.h>
#include <node.hpp>
#include <nlohmann/json.hpp>
#include <utility.hpp>


#include <pcapplusplus/PcapLiveDeviceList.h>

#include <pcapplusplus/PcapLiveDevice.h>
#include <pcapplusplus/Device.h>
#include <pcapplusplus/SystemUtils.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/EthLayer.h>
#include <pcapplusplus/PcapFilter.h>
#include <PacketState.hpp>
// #include "PcapFilter.h"
// #include "PcapFileDevice.h"

namespace PcapEditor
{


    class NodeNullptr : public Node {
    public:
        NodeNullptr() : Node("hex.builtin.nodes.constants.nullptr.header", { Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "") }) { }

        void process() override {
            this->setBufferOnOutput(0, {});
        }
    };

    class NodeBuffer : public Node {
    public:
        NodeBuffer() : Node("hex.builtin.nodes.constants.buffer.header", { Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "") }) { }

        void drawNode() override {
            constexpr int StepSize = 1, FastStepSize = 10;

            ImGui::PushItemWidth(100);
            ImGui::InputScalar("hex.builtin.nodes.constants.buffer.size", ImGuiDataType_U32, &this->m_size, &StepSize, &FastStepSize);
            ImGui::PopItemWidth();
        }

        void process() override {
            if (this->m_buffer.size() != this->m_size)
                this->m_buffer.resize(this->m_size, 0x00);

            this->setBufferOnOutput(0, this->m_buffer);
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["size"] = this->m_size;
            j["data"] = this->m_buffer;
        }

        void load(nlohmann::json &j) override {
            this->m_size   = j["size"];
            this->m_buffer = j["data"].get<std::vector<u8>>();
        }

    private:
        u32 m_size = 1;
        std::vector<u8> m_buffer;
    };

    class NodeString : public Node {
    public:
        NodeString() : Node("hex.builtin.nodes.constants.string.header", { Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "") }) {
            this->m_value.resize(0xFFF, 0x00);
        }

        void drawNode() override {
            ImGui::PushItemWidth(100);
            ImGui::InputText("##string", reinterpret_cast<char *>(this->m_value.data()), this->m_value.size() - 1);
            ImGui::PopItemWidth();
        }

        void process() override {
            std::vector<u8> output(std::strlen(this->m_value.c_str()) + 1, 0x00);
            std::strcpy(reinterpret_cast<char *>(output.data()), this->m_value.c_str());

            output.pop_back();

            this->setBufferOnOutput(0, output);
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["data"] = this->m_value;
        }

        void load(nlohmann::json &j) override {
            this->m_value = j["data"];
        }

    private:
        std::string m_value;
    };

    
    class NodeInteger : public Node {
    public:
        NodeInteger() : Node("hex.builtin.nodes.constants.int.header", { Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(100);
            ImGui::InputHexadecimal("##integer_value", &this->m_value);
            ImGui::PopItemWidth();
        }

        void process() override {
            this->setIntegerOnOutput(0, this->m_value);
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["data"] = this->m_value;
        }

        void load(nlohmann::json &j) override {
            this->m_value = j["data"];
        }

    private:
        u64 m_value = 0;
    };

    class NodeFloat : public Node {
    public:
        NodeFloat() : Node("hex.builtin.nodes.constants.float.header", { Attribute(Attribute::IOType::Out, Attribute::Type::Float, "") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(100);
            ImGui::InputScalar("##floatValue", ImGuiDataType_Float, &this->m_value, nullptr, nullptr, "%f", ImGuiInputTextFlags_CharsDecimal);
            ImGui::PopItemWidth();
        }

        void process() override {
            this->setFloatOnOutput(0, this->m_value);
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["data"] = this->m_value;
        }

        void load(nlohmann::json &j) override {
            this->m_value = j["data"];
        }

    private:
        float m_value = 0;
    };

    class NodeRGBA8 : public Node {
    public:
        NodeRGBA8() : Node("hex.builtin.nodes.constants.rgba8.header",
                          { Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.constants.rgba8.output.r"),
                              Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.constants.rgba8.output.g"),
                              Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.constants.rgba8.output.b"),
                              Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.constants.rgba8.output.a") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(200);
            ImGui::ColorPicker4("##colorPicker", &this->m_color.Value.x, ImGuiColorEditFlags_AlphaBar);
            ImGui::PopItemWidth();
        }

        void process() override {
            this->setBufferOnOutput(0, utility::toBytes<u64>(this->m_color.Value.x * 0xFF));
            this->setBufferOnOutput(1, utility::toBytes<u64>(this->m_color.Value.y * 0xFF));
            this->setBufferOnOutput(2, utility::toBytes<u64>(this->m_color.Value.z * 0xFF));
            this->setBufferOnOutput(3, utility::toBytes<u64>(this->m_color.Value.w * 0xFF));
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["data"]      = nlohmann::json::object();
            j["data"]["r"] = this->m_color.Value.x;
            j["data"]["g"] = this->m_color.Value.y;
            j["data"]["b"] = this->m_color.Value.z;
            j["data"]["a"] = this->m_color.Value.w;
        }

        void load(nlohmann::json &j) override {
            this->m_color = ImVec4(j["data"]["r"], j["data"]["g"], j["data"]["b"], j["data"]["a"]);
        }

    private:
        ImColor m_color;
    };

    class NodeComment : public Node {
    public:
        NodeComment() : Node("hex.builtin.nodes.constants.comment.header", {}) {
            this->m_comment.resize(0xFFF, 0x00);
        }

        void drawNode() override {
            ImGui::InputTextMultiline("##string", reinterpret_cast<char *>(this->m_comment.data()), this->m_comment.size() - 1, ImVec2(150, 100));
        }

        void process() override {
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["comment"] = this->m_comment;
        }

        void load(nlohmann::json &j) override {
            this->m_comment = j["comment"];
        }

    private:
        std::string m_comment;
    };


    class NodeDisplayInteger : public Node {
    public:
        NodeDisplayInteger() : Node("hex.builtin.nodes.display.int.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(150);
            if (this->m_value.has_value())
                ImGui::TextFormatted("0x{0:X}", this->m_value.value());
            else
                ImGui::TextUnformatted("???");
            ImGui::PopItemWidth();
        }

        void process() override {
            this->m_value.reset();
            auto input = this->getIntegerOnInput(0);

            this->m_value = input;
        }

    private:
        std::optional<u64> m_value;
    };

    class NodeDisplayBuffer : public Node {
    public:
        NodeDisplayBuffer() : Node("hex.builtin.nodes.display.buffer.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(150);
            if (this->m_value.has_value())
            {
                auto v = this->m_value.value() ;
                
                ImGui::TextFormatted("{0}", utility::hexStr(v.data(), v.size()));
            }
                
            else
                ImGui::TextUnformatted("???");
            ImGui::PopItemWidth();
        }

        void process() override {
            this->m_value.reset();
            auto input = this->getBufferOnInput(0);

            this->m_value = input;
        }

    private:
        std::optional<std::vector<u8> > m_value;
    };
    class NodeDisplayString : public Node {
    public:
        NodeDisplayString() : Node("hex.builtin.nodes.display.string.header", { Attribute(Attribute::IOType::In, Attribute::Type::String, "hex.builtin.nodes.common.input") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(150);
            if (this->m_value.has_value())
            {
                    ImGui::TextFormatted("{0}", this->m_value.value());
            }
                
            else
                ImGui::TextUnformatted("???");
            ImGui::PopItemWidth();
        }

        void process() override {
            this->m_value.reset();
            auto input = this->getStringOnInput(0);

            this->m_value = input;
        }

    private:
        std::optional<std::string > m_value;
    };

    class NodeDisplayFloat : public Node {
    public:
        NodeDisplayFloat() : Node("hex.builtin.nodes.display.float.header", { Attribute(Attribute::IOType::In, Attribute::Type::Float, "hex.builtin.nodes.common.input") }) { }

        void drawNode() override {
            ImGui::PushItemWidth(150);
            if (this->m_value.has_value())
                ImGui::TextFormatted("{0}", this->m_value.value());
            else
                ImGui::TextUnformatted("???");
            ImGui::PopItemWidth();
        }

        void process() override {
            this->m_value.reset();
            auto input = this->getFloatOnInput(0);

            this->m_value = input;
        }

    private:
        std::optional<float> m_value;
    };
    


    class NodeBitwiseNOT : public Node {
    public:
        NodeBitwiseNOT() : Node("hex.builtin.nodes.bitwise.not.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto input = this->getBufferOnInput(0);

            std::vector<u8> output = input;
            for (auto &byte : output)
                byte = ~byte;

            this->setBufferOnOutput(1, output);
        }
    };

    class NodeBitwiseAND : public Node {
    public:
        NodeBitwiseAND() : Node("hex.builtin.nodes.bitwise.and.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getBufferOnInput(0);
            auto inputB = this->getBufferOnInput(1);

            std::vector<u8> output(std::min(inputA.size(), inputB.size()), 0x00);

            for (u32 i = 0; i < output.size(); i++)
                output[i] = inputA[i] & inputB[i];

            this->setBufferOnOutput(2, output);
        }
    };

    class NodeBitwiseOR : public Node {
    public:
        NodeBitwiseOR() : Node("hex.builtin.nodes.bitwise.or.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getBufferOnInput(0);
            auto inputB = this->getBufferOnInput(1);

            std::vector<u8> output(std::min(inputA.size(), inputB.size()), 0x00);

            for (u32 i = 0; i < output.size(); i++)
                output[i] = inputA[i] | inputB[i];

            this->setBufferOnOutput(2, output);
        }
    };

    class NodeBitwiseXOR : public Node {
    public:
        NodeBitwiseXOR() : Node("hex.builtin.nodes.bitwise.xor.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getBufferOnInput(0);
            auto inputB = this->getBufferOnInput(1);

            std::vector<u8> output(std::min(inputA.size(), inputB.size()), 0x00);

            for (u32 i = 0; i < output.size(); i++)
                output[i] = inputA[i] ^ inputB[i];

            this->setBufferOnOutput(2, output);
        }
    };



    class NodeWriteData : public Node {
    public:
        NodeWriteData() : Node("hex.builtin.nodes.data_access.write.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.data_access.write.address"), Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.data_access.write.data") }) { }

        void process() override {
            auto address = this->getIntegerOnInput(0);
            auto data    = this->getBufferOnInput(1);

            this->setOverlayData(address, data);
        }
    };


    class NodeCastIntegerToBuffer : public Node {
    public:
        NodeCastIntegerToBuffer() : Node("hex.builtin.nodes.casting.int_to_buffer.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto input = this->getIntegerOnInput(0);

            std::vector<u8> output(sizeof(u64), 0x00);
            std::memcpy(output.data(), &input, sizeof(u64));

            this->setBufferOnOutput(1, output);
        }
    };

    class NodeCastBufferToInteger : public Node {
    public:
        NodeCastBufferToInteger() : Node("hex.builtin.nodes.casting.buffer_to_int.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input"), Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto input = this->getBufferOnInput(0);

            if (input.size() == 0 || input.size() > sizeof(u64))
                throwNodeError("Buffer is empty or bigger than 64 bits");

            u64 output = 0;
            std::memcpy(&output, input.data(), input.size());

            this->setIntegerOnOutput(1, output);
        }
    };

    class NodeArithmeticAdd : public Node {
    public:
        NodeArithmeticAdd() : Node("hex.builtin.nodes.arithmetic.add.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            auto output = inputA + inputB;

            this->setIntegerOnOutput(2, output);
        }
    };

    class NodeArithmeticSubtract : public Node {
    public:
        NodeArithmeticSubtract() : Node("hex.builtin.nodes.arithmetic.sub.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            auto output = inputA - inputB;

            this->setIntegerOnOutput(2, output);
        }
    };

    class NodeArithmeticMultiply : public Node {
    public:
        NodeArithmeticMultiply() : Node("hex.builtin.nodes.arithmetic.mul.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            auto output = inputA * inputB;

            this->setIntegerOnOutput(2, output);
        }
    };

    class NodeArithmeticDivide : public Node {
    public:
        NodeArithmeticDivide() : Node("hex.builtin.nodes.arithmetic.div.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            if (inputB == 0)
                throwNodeError("Division by zero");

            auto output = inputA / inputB;

            this->setIntegerOnOutput(2, output);
        }
    };

    class NodeArithmeticModulus : public Node {
    public:
        NodeArithmeticModulus() : Node("hex.builtin.nodes.arithmetic.mod.header", { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            if (inputB == 0)
                throwNodeError("Division by zero");

            auto output = inputA % inputB;

            this->setIntegerOnOutput(2, output);
        }
    };

    class NodeBufferCombine : public Node {
    public:
        NodeBufferCombine() : Node("hex.builtin.nodes.buffer.combine.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.a"), Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.common.input.b"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getBufferOnInput(0);
            auto inputB = this->getBufferOnInput(1);

            auto &output = inputA;
            std::copy(inputB.begin(), inputB.end(), std::back_inserter(output));

            this->setBufferOnOutput(2, output);
        }
    };

    class NodeBufferSlice : public Node {
    public:
        NodeBufferSlice() : Node("hex.builtin.nodes.buffer.slice.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.buffer.slice.input.buffer"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.buffer.slice.input.from"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.buffer.slice.input.to"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto input = this->getBufferOnInput(0);
            auto from  = this->getIntegerOnInput(1);
            auto to    = this->getIntegerOnInput(2);

            if (from < 0 || from >= input.size())
                throwNodeError("'from' input out of range");
            if (to < 0 || from >= input.size())
                throwNodeError("'to' input out of range");
            if (to <= from)
                throwNodeError("'to' input needs to be greater than 'from' input");

            this->setBufferOnOutput(3, std::vector(input.begin() + from, input.begin() + to));
        }
    };

    class NodeBufferRepeat : public Node {
    public:
        NodeBufferRepeat() : Node("hex.builtin.nodes.buffer.repeat.header", { Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.buffer.repeat.input.buffer"), Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.buffer.repeat.input.count"), Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto buffer = this->getBufferOnInput(0);
            auto count  = this->getIntegerOnInput(1);

            std::vector<u8> output;
            output.resize(buffer.size() * count);

            for (u32 i = 0; i < count; i++)
                std::copy(buffer.begin(), buffer.end(), output.begin() + buffer.size() * i);

            this->setBufferOnOutput(2, output);
        }
    };

    class NodeIf : public Node {
    public:
        NodeIf() : Node("hex.builtin.nodes.control_flow.if.header",
                       { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.control_flow.if.condition"),
                           Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.control_flow.if.true"),
                           Attribute(Attribute::IOType::In, Attribute::Type::Buffer, "hex.builtin.nodes.control_flow.if.false"),
                           Attribute(Attribute::IOType::Out, Attribute::Type::Buffer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto cond      = this->getIntegerOnInput(0);
            auto trueData  = this->getBufferOnInput(1);
            auto falseData = this->getBufferOnInput(2);

            if (cond != 0)
                this->setBufferOnOutput(3, trueData);
            else
                this->setBufferOnOutput(3, falseData);
        }
    };

    class NodeEquals : public Node {
    public:
        NodeEquals() : Node("hex.builtin.nodes.control_flow.equals.header",
                           { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"),
                               Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"),
                               Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            this->setIntegerOnOutput(2, inputA == inputB);
        }
    };

    class NodeNot : public Node {
    public:
        NodeNot() : Node("hex.builtin.nodes.control_flow.not.header",
                        { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input"),
                            Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto input = this->getIntegerOnInput(0);

            this->setIntegerOnOutput(1, !input);
        }
    };

    class NodeGreaterThan : public Node {
    public:
        NodeGreaterThan() : Node("hex.builtin.nodes.control_flow.gt.header",
                                { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"),
                                    Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"),
                                    Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            this->setIntegerOnOutput(2, inputA > inputB);
        }
    };

    class NodeLessThan : public Node {
    public:
        NodeLessThan() : Node("hex.builtin.nodes.control_flow.lt.header",
                             { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"),
                                 Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"),
                                 Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            this->setIntegerOnOutput(2, inputA < inputB);
        }
    };

    class NodeBoolAND : public Node {
    public:
        NodeBoolAND() : Node("hex.builtin.nodes.control_flow.and.header",
                            { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"),
                                Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"),
                                Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            this->setIntegerOnOutput(2, inputA && inputB);
        }
    };

    class NodeBoolOR : public Node {
    public:
        NodeBoolOR() : Node("hex.builtin.nodes.control_flow.or.header",
                           { Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.a"),
                               Attribute(Attribute::IOType::In, Attribute::Type::Integer, "hex.builtin.nodes.common.input.b"),
                               Attribute(Attribute::IOType::Out, Attribute::Type::Integer, "hex.builtin.nodes.common.output") }) { }

        void process() override {
            auto inputA = this->getIntegerOnInput(0);
            auto inputB = this->getIntegerOnInput(1);

            this->setIntegerOnOutput(2, inputA || inputB);
        }
    };


    class NodePortFilter : public Node {
    public:
        NodePortFilter() : Node("hex.builtin.nodes.net.Filter.header", { Attribute(Attribute::IOType::Out, Attribute::Type::Pointer, "") }) {
            // m_portFilter = new pcpp::PortFilter(m_value, pcpp::SRC_OR_DST);
         }

        void drawNode() override {
            ImGui::PushItemWidth(100);
            // ImGui::InputHexadecimal("##port_value", &this->m_value);
            ImGui::InputInt("##port_value", &this->m_value);
            ImGui::PopItemWidth();
        }

        void process() override {
            m_portFilter.setPort(m_value);
            this->setTOnOutput<pcpp::GeneralFilter>(0, &m_portFilter);
        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            j["data"] = this->m_value;
        }

        void load(nlohmann::json &j) override {
            this->m_value = j["data"];
        }
        ~NodePortFilter(){
            // delete m_portFilter;
        }

    private:
        int m_value = 80;
        pcpp::PortFilter m_portFilter = {0, pcpp::SRC_OR_DST};
    };
    
    class NodeDisPlayStats : public Node {
    public:
        NodeDisPlayStats() : Node("hex.builtin.nodes.pcap.stats.header",
                           { Attribute(Attribute::IOType::In, Attribute::Type::Pointer, "hex.builtin.nodes.common.input.a") }) {
                                    
                                }

        void drawNode() override {

            ImGui::Text(output.c_str());
            
            
        }

        void process() override {
            
            // p_stats= this->getStatsOnInput(0);
            p_stats= this->getTOnInput<pcpp::Stats, Attribute::Type::Pointer>(0);
            output = p_stats->printToConsole();
            
            
        }
    private:
        pcpp::Stats* p_stats;
        std::string output;
        
    };

    // class NodeDisplayPacket: public Node{
    // public:
    //     NodeDisplayPacket():Node("hex.builtin.nodes.pcap.pointer.header",
    //                        { Attribute(Attribute::IOType::In, Attribute::Type::Pointer, "hex.builtin.nodes.common.input.a") }) {
    //     }

    //     void drawNode() override {
    //         ImGui::Text(output.c_str());            
    //     }

    //     void process() override {            
    //         // p_stats= this->getStatsOnInput(0);
    //         p_packet= this->getTOnInput<pcpp::Packet, Attribute::Type::Pointer>(0);
    //         output = p_packet->toString();
    //     }
    // private:
    //     std::string output;
    //     pcpp::Packet *p_packet;
    // };
    
    class NodeFilterOR : public Node {
    public:
        NodeFilterOR() : Node("hex.builtin.nodes.filter.or.header",
                           { Attribute(Attribute::IOType::In, Attribute::Type::Pointer, "hex.builtin.nodes.common.input.a"),
                               Attribute(Attribute::IOType::In, Attribute::Type::Pointer, "hex.builtin.nodes.common.input.b"),
                               Attribute(Attribute::IOType::Out, Attribute::Type::Pointer, "hex.builtin.nodes.common.output") }) {
                                    
                                }

        void process() override {
            std::vector<pcpp::GeneralFilter*> FilterVec;
            auto inputA = this->getTOnInput<pcpp::GeneralFilter, Attribute::Type::Pointer>(0);
            auto inputB = this->getTOnInput<pcpp::GeneralFilter, Attribute::Type::Pointer>(1);
            FilterVec.push_back(inputA);
            FilterVec.push_back(inputB);
            orFilter = FilterVec;     

            // std::string filterAsString;
		    // orFilter.parseToString(filterAsString);
            this->setTOnOutput<pcpp::GeneralFilter>(2, &orFilter);
        }
    private:
        std::vector<pcpp::GeneralFilter*> m_FilterList;
        pcpp::OrFilter orFilter;
        
    };

    class NodePcap : public Node{
    public:
        NodePcap() : Node("hex.builtin.nodes.device.pcap.header", 
            {
                Attribute(Attribute::IOType::Out, Attribute::Type::String, "Interface Info"),
                Attribute(Attribute::IOType::Out, Attribute::Type::Pointer, "Packet Statistic struct") ,
                Attribute(Attribute::IOType::In, Attribute::Type::Pointer, "filter") }) { 
            this->m_deviceList = pcpp::PcapLiveDeviceList::getInstance().getPcapLiveDevicesList();
            open(item_current_idx, item_current_idx);
        }
        void closeall(){
            for(int i = 0; i < m_deviceList.size(); i++){
                close(item_current_idx);
            }
        }
        ~NodePcap(){
            closeall();
            
        }
        struct if_info{
            std::string name;
            std::string desc;
            std::string mac_address;
            std::string default_gw;
            std::string mtu;
            void clear(){name.clear();desc.clear();mac_address.clear();default_gw.clear();mtu.clear();}
            std::string get_if_info(auto&& select_dev){
                try{
                    std::stringstream ss;
                    ss
                    << "Interface info:" << std::endl
                    << "   Interface name:        " << select_dev->getName() << std::endl // get interface name
                    << "   Interface description: " << select_dev->getDesc() << std::endl // get interface description
                    << "   MAC address:           " << select_dev->getMacAddress() << std::endl // get interface MAC address
                    << "   Default gateway:       " << select_dev->getDefaultGateway() << std::endl // get default gateway
                    << "   Interface MTU:         " << select_dev->getMtu() << std::endl; // get interface MTU
                    return ss.str();
                }
                catch(const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                    return "error occur!";
                }
                

            }
        };
        
        
        void drawNode() override {
            ImGui::PushItemWidth(100);
            if(this->m_deviceList.size() > 0){
                if (ImGui::BeginCombo("interface", m_deviceList[item_current_idx]->getName().c_str()))
                {
                     for (int i = 0; i < m_deviceList.size(); i++) {
                            bool is_selected = (this->item_current_idx == i);
                            auto cur_str = m_deviceList[i]->getName()  +  m_deviceList[i]->getIPv4Address().toString();
                            if (ImGui::Selectable(cur_str.c_str(), is_selected))
                                open(item_current_idx, i);
                            if (is_selected){
                                ImGui::SetItemDefaultFocus(); 
                            }
                        }
                    ImGui::EndCombo();
                }
                std::string output = utility::format("cur if:{0}", (m_deviceList[item_current_idx]->getName()));
                ImGui::Text(output.c_str());
                
            }

            ImGui::PopItemWidth();
        }

        void close(int i ){
            
            try{
                select_dev = m_deviceList[i];
                select_dev->stopCapture();
                select_dev->close();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
        void open(int old, int i ){
            
            try
            {
                close(old);
                item_current_idx = i;
                select_dev = m_deviceList[item_current_idx];
                select_dev->open();
                select_dev->startCapture(onPacketArrives, &stats);
                stats.clear();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }

            
        }
        static void onPacketArrives(pcpp::RawPacket* packet, pcpp::PcapLiveDevice* dev, void* cookie)
        {
            // extract the stats object form the cookie
            pcpp::PacketStats* stats = (pcpp::PacketStats*)cookie;

            // parsed the raw packet
            pcpp::Packet parsedPacket(packet);
            // collect stats from packet
            stats->consumePacket(parsedPacket);
        }

        void process() override {

            // this->setStringOnOutput(0, if_information.get_if_info(select_dev)); 
            // this->setStatsOnOutput(1, &stats);
            result = if_information.get_if_info(select_dev);
            this->setStringOnOutput(0, result); 
            this->setTOnOutput<pcpp::Stats>(1, &stats);
            try{

                filter = this->getTOnInput<pcpp::GeneralFilter, Attribute::Type::Pointer>(2);
                // std::cout<<filter;
                std::string filterAsString;
                filter->parseToString(filterAsString);
                if (!select_dev->setFilter(*filter))
                {
                    
                    std::cerr << "Couldn't set the filter '" << filterAsString << "' for the device";
                }
            }
            catch(const std::exception& e){
                throwNodeError(utility::format("NIC error'{0}'", e.what()));;
            }

        }

        void store(nlohmann::json &j) override {
            j = nlohmann::json::object();

            // j["size"] = this->m_size;
            // j["data"] = this->m_buffer;
        }

        void load(nlohmann::json &j) override {
            // this->m_size   = j["size"];
            // this->m_buffer = j["data"].get<std::vector<u8>>();
        }

    private:
        bool m_open = false;
        bool is_dev_open = false;
        u32 item_current_idx = 0;
        // std::vector<u8> m_buffer;
        u64 m_recivedPacketSize = 0;
        std::vector<pcpp::PcapLiveDevice*> m_deviceList;
        pcpp::PcapLiveDevice* select_dev;
        pcpp::PacketStats stats;
        pcpp::GeneralFilter *filter;
        if_info if_information;
        std::string result;
    };
    
void registerNodes() {
        utility::add<NodeInteger>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.int");
        utility::add<NodeFloat>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.float");
        utility::add<NodeNullptr>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.nullptr");
        utility::add<NodeBuffer>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.buffer");
        utility::add<NodeString>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.string");
        utility::add<NodeRGBA8>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.rgba8");
        utility::add<NodeComment>("hex.builtin.nodes.constants", "hex.builtin.nodes.constants.comment");
        

        utility::add<NodeDisPlayStats>("hex.builtin.nodes.display", "hex.builtin.nodes.display.stats");

        utility::add<NodeDisplayInteger>("hex.builtin.nodes.display", "hex.builtin.nodes.display.int");
        utility::add<NodeDisplayFloat>("hex.builtin.nodes.display", "hex.builtin.nodes.display.float");
        utility::add<NodeDisplayBuffer>("hex.builtin.nodes.display", "hex.builtin.nodes.display.buffer");
        utility::add<NodeDisplayString>("hex.builtin.nodes.display", "hex.builtin.nodes.display.string");


        utility::add<NodeCastIntegerToBuffer>("hex.builtin.nodes.casting", "hex.builtin.nodes.casting.int_to_buffer");
        utility::add<NodeCastBufferToInteger>("hex.builtin.nodes.casting", "hex.builtin.nodes.casting.buffer_to_int");

        utility::add<NodeArithmeticAdd>("hex.builtin.nodes.arithmetic", "hex.builtin.nodes.arithmetic.add");
        utility::add<NodeArithmeticSubtract>("hex.builtin.nodes.arithmetic", "hex.builtin.nodes.arithmetic.sub");
        utility::add<NodeArithmeticMultiply>("hex.builtin.nodes.arithmetic", "hex.builtin.nodes.arithmetic.mul");
        utility::add<NodeArithmeticDivide>("hex.builtin.nodes.arithmetic", "hex.builtin.nodes.arithmetic.div");
        utility::add<NodeArithmeticModulus>("hex.builtin.nodes.arithmetic", "hex.builtin.nodes.arithmetic.mod");

        utility::add<NodeBufferCombine>("hex.builtin.nodes.buffer", "hex.builtin.nodes.buffer.combine");
        utility::add<NodeBufferSlice>("hex.builtin.nodes.buffer", "hex.builtin.nodes.buffer.slice");
        utility::add<NodeBufferRepeat>("hex.builtin.nodes.buffer", "hex.builtin.nodes.buffer.repeat");

        utility::add<NodeIf>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.if");
        utility::add<NodeEquals>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.equals");
        utility::add<NodeNot>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.not");
        utility::add<NodeGreaterThan>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.gt");
        utility::add<NodeLessThan>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.lt");
        utility::add<NodeBoolAND>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.and");
        utility::add<NodeBoolOR>("hex.builtin.nodes.control_flow", "hex.builtin.nodes.control_flow.or");

        utility::add<NodeBitwiseAND>("hex.builtin.nodes.bitwise", "hex.builtin.nodes.bitwise.and");
        utility::add<NodeBitwiseOR>("hex.builtin.nodes.bitwise", "hex.builtin.nodes.bitwise.or");
        utility::add<NodeBitwiseXOR>("hex.builtin.nodes.bitwise", "hex.builtin.nodes.bitwise.xor");
        utility::add<NodeBitwiseNOT>("hex.builtin.nodes.bitwise", "hex.builtin.nodes.bitwise.not");

        utility::add<NodeFilterOR>("hex.builtin.nodes.filter", "hex.builtin.nodes.filter.or");
        
        utility::add<NodePortFilter>("hex.builtin.nodes.filter", "hex.builtin.nodes.filter.portfilter");
        utility::add<NodePcap>("hex.builtin.nodes.device", "hex.builtin.nodes.device.pcap");


    }      
    

} // namespace PcapEditor


#pragma once
#include <iostream>
#include <defination.hpp>
#include <attribute.hpp>

#include <imgui.h>
#include <set>
#include <string_view>
#include <vector>
#include <cstdio>
#include <nlohmann/json_fwd.hpp>
#include <utility.hpp>

#include <pcapplusplus/PcapFilter.h>

namespace PcapEditor {
    class Overlay {
    public:
        Overlay() { }

        void setAddress(u64 address) { this->m_address = address; }
        [[nodiscard]] u64 getAddress() const { return this->m_address; }

        [[nodiscard]] u64 getSize() const { return this->m_data.size(); }
        [[nodiscard]] std::vector<u8> &getData() { return this->m_data; }

    private:
        u64 m_address = 0;
        std::vector<u8> m_data;
    };
    class Node {
    public:
        Node(std::string unlocalizedTitle, std::vector<Attribute> attributes);

        virtual ~Node() = default;

        [[nodiscard]] u32 getId() const { return this->m_id; }
        void setId(u32 id) { this->m_id = id; }

        [[nodiscard]] const std::string &getUnlocalizedName() const { return this->m_unlocalizedName; }
        void setUnlocalizedName(const std::string &unlocalizedName) { this->m_unlocalizedName = unlocalizedName; }

        [[nodiscard]] const std::string &getUnlocalizedTitle() const { return this->m_unlocalizedTitle; }
        [[nodiscard]] std::vector<Attribute> &getAttributes() { return this->m_attributes; }

        void setCurrentOverlay(Overlay *overlay) {
            this->m_overlay = overlay;
        }

        virtual void drawNode(){} 
        virtual void process() = 0;


        virtual void store(nlohmann::json &j) { }
        virtual void load(nlohmann::json &j) { }

        using NodeError = std::pair<Node *, std::string>;

        void resetOutputData() {
            for (auto &attribute : this->m_attributes)
                attribute.getOutputData().reset();
        }

        void resetProcessedInputs() {
            this->m_processedInputs.clear();
        }

        static void setIdCounter(u32 id) {
            if (id > Node::s_idCounter)
                Node::s_idCounter = id;
        }

    private:
        u32 m_id;
        std::string m_unlocalizedTitle, m_unlocalizedName;
        std::vector<Attribute> m_attributes;
        std::set<u32> m_processedInputs;
        Overlay *m_overlay = nullptr;

        static u32 s_idCounter;

        Attribute *getConnectedInputAttribute(u32 index) {
            if (index >= this->getAttributes().size())
                throw std::runtime_error("Attribute index out of bounds!");

            auto &connectedAttribute = this->getAttributes()[index].getConnectedAttributes();

            if (connectedAttribute.empty())
                return nullptr;

            return connectedAttribute.begin()->second;
        }

        void markInputProcessed(u32 index) {
            const auto &[iter, inserted] = this->m_processedInputs.insert(index);
            if (!inserted)
                throwNodeError("Recursion detected!");
        }

    protected:
        [[noreturn]] void throwNodeError(const std::string &message) {
            throw NodeError(this, message);
        }

        std::vector<u8> getBufferOnInput(u32 index);
        u64 getIntegerOnInput(u32 index);
        float getFloatOnInput(u32 index);
        pcpp::GeneralFilter* getFilterOnInput(u32 index);


        void setBufferOnOutput(u32 index, std::vector<u8> data);
        void setIntegerOnOutput(u32 index, u64 integer);
        void setFloatOnOutput(u32 index, float floatingPoint);
        void setFilterOnOutput(u32 index, pcpp::GeneralFilter* filter);

        void setOverlayData(u64 address, const std::vector<u8> &data);
    };

   
}
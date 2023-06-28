#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
using namespace std::chrono;

// Send the request to the daemon
json sendRequest(json requestBody)
{
    auto res = cpr::Post(cpr::Url{"http://127.0.0.1:19091/json_rpc"},
                         cpr::Body{requestBody.dump()},
                         cpr::Header{{"Content-Type", "application/json"}});
    if (!res.text.size())
        throw std::runtime_error{"Data retrieval timeout, and daemon connection failed"};

    // Parse the response
    json response = json::parse(res.text);
    auto details = response["result"];
    return details;
}

int main()
{
    // Open the file object
    std::ofstream fout;
    fout.open("tx_flash_history.csv");

    int start_height = 2359521;

    // Loop for run the blocks upto it satisfy the end-height
    for (; start_height <= 2359858; start_height++)
    {
        // Create json object(get_block) for the cpr params input
        json block_fast = {
            {"jsonrpc", "2.0"},
            {"id", "0"},
            {"method", "get_block"},
            {"params", {{"height", std::to_string(start_height)}}}};

        // Send the request to the daemon
        auto details = sendRequest(block_fast);

        if (details["status"] == "Failed")
            throw std::runtime_error{"Daemon unexpectedly returned zero blocks and status failed"};

        // Get the transaction hash from the block and process the transactions
        for (auto txs : details["tx_hashes"])
        {
            std::string txid = txs;
            // curl -X POST http://127.0.0.1:19091/get_transactions -d '{"txs_hashes":["d6e48158472848e6687173a91ae6eebfa3e1d778e65252ee99d7515d63090408"]}' -H 'Content-Type: application/json'

            // Create json object(get_transactions) for the cpr params input
            json get_txs = {
                {"jsonrpc", "2.0"},
                {"id", "0"},
                {"method", "get_transactions"},
                {"params", {{"txs_hashes", {txid}}, {"tx_extra", true}, {"decode_as_json", true}, {"prune", true}}}};

            // Send the request to the daemon
            auto tx_details = sendRequest(get_txs);

            // Process the transactions details and get the required value and add into the file
            for (auto &data : tx_details["txs"])
            {
                std::string it = data["as_json"];
                data["as_json"] = json::parse(it);
                if (data["extra"].contains("burn_amount"))
                {
                    std::time_t timestamp = data["block_timestamp"];

                    // Convert the timestamp value to a tm struct in UTC
                    std::tm *utcTime = std::gmtime(&timestamp);

                    // Convert the tm struct to a string representation
                    char buffer[80];
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", utcTime);

                    double burn = data["extra"]["burn_amount"];
                    double fee = data["as_json"]["rct_signatures"]["txnFee"];
                    double weight = data["size"];
                    fout << data["block_height"] << "," << data["tx_hash"] << "," << weight / 1000 << "kb," << fee / 1000000000 << "," << burn / 1000000000 << "," << buffer << "\n";
                }
            }
        }
    }
    std::cout << "Updated Succesfully\n";
    fout.close();
    return 0;
}
#include <iostream>
#include <fstream>
#include "cpr/cpr.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <map>

using json = nlohmann::json;
using namespace std::chrono;

json belnetVersion = {0, 9, 7};
json storageVersion = {2, 3, 0};
json daemonVersion = {5, 0, 2};

int belnet0_9_7 = 0;
int daemon5_0_2 = 0;
int storage2_3_0 = 0;

int daemonNewDecom = 0;
int daemonOldDecom = 0;
std::map<std::string, int> oldContributorList;
std::vector<std::string> oldContributors;

void checkBelnetVersion(json belnetV, json belnetAV)
{
    for (int i = 0; i < 3; i++)
    {
        if (belnetV[i] == belnetAV[i])
        {
            continue;
        }
        else
        {
            return;
        }
    }
    belnet0_9_7++;
}

void checkDaemonVersion(json mnData, json daemonV, json daemonAV, std::ofstream &_fout)
{
    for (int i = 0; i < 3; i++)
    {
        if (daemonV[i] == daemonAV[i])
        {
            continue;
        }
        else
        {
            _fout << mnData["public_ip"] << "," << mnData["master_node_pubkey"] << "," << mnData["operator_address"] << std::endl;
            oldContributors.push_back(mnData["operator_address"]);
            ++oldContributorList[mnData["operator_address"]];
            if (!mnData["active"])
            {
                daemonOldDecom++;
            }
            return;
        }
    }
    daemon5_0_2++;
    if (!mnData["active"])
    {
        daemonNewDecom++;
    }
}
void checkStorageVersion(json storageV, json storageAV)
{
    for (int i = 0; i < 3; i++)
    {
        if (storageV[i] == storageAV[i])
        {
            continue;
        }
        else
        {
            return;
        }
    }
    storage2_3_0++;
}

//checking the masternode old versions also the check the decommission lists
void masterNodeversionMonitor(json resultTx)
{
    // This is for not the old versions list of address and their nodes
    std::ofstream fout;
    std::ofstream oldConList;
    fout.open("olddaemns.csv"); //     fout.open("olddaemns.csv", std::ios::app);
    oldConList.open("oldContributorList.csv");

    // This file for the decommission list of all the master nodes
    std::ofstream decomList;
    decomList.open("decommissionList.csv");

    // This file for the decommission list based on address
    std::ofstream decomListCum;
    decomListCum.open("decommissionListCumulative.csv");
    std::map<std::string, int> decomListCumMap;

    json decomissionNodes;

    std::cout << "Total master-nodes : " << resultTx["result"]["master_node_states"].size() << std::endl;
    for (auto master_node_data : resultTx["result"]["master_node_states"])
    {
        checkBelnetVersion(master_node_data["belnet_version"], belnetVersion);
        checkDaemonVersion(master_node_data, master_node_data["master_node_version"], daemonVersion, fout);
        checkStorageVersion(master_node_data["storage_server_version"], storageVersion);
        if (!master_node_data["active"])
        {
            decomissionNodes.push_back(master_node_data);
        }
    }

    std::cout << "daemon(5.0.2)  : " << daemon5_0_2 << std::endl;
    std::cout << "daemon(< 5.0.2): " << oldContributors.size() << std::endl << std::endl;
    
    std::cout << "decomissionNodes.size() : " << decomissionNodes.size() << std::endl;
    std::cout << "daemonNewDecom    : " << daemonNewDecom << std::endl;
    std::cout << "daemonOldDecom    : " << daemonOldDecom << std::endl << std::endl;

    for (auto address : decomissionNodes)
    {
        if (address["operator_address"] == "bxbz3Ynqzu9WYHXBnTVL7bP1UhLCduRmH9vRH32tqcRTSksbQDjqEoweZDQWsiNKL8QHBtGzhPK3fiayLWAReAjD1rrBuinCh")
            std::cout << "Our DecomissionNodes address : " << address["operator_address"] << " ,IP : " << address["public_ip"] << " and mn-key : " << address["pubkey_ed25519"] << std::endl;
        decomList << address["operator_address"] << "," << address["public_ip"] << "," << address["pubkey_ed25519"] << std::endl;
        ++decomListCumMap[address["operator_address"]];
    }
    assert((oldContributors.size() + daemon5_0_2) == resultTx["result"]["master_node_states"].size());

    int checkdecom =0;
    for (auto it = decomListCumMap.begin(); it!= decomListCumMap.end(); it++)
    {
        decomListCum << it->first << "," << it->second << std::endl;
        checkdecom += it->second;
    }

    assert(checkdecom == decomissionNodes.size());

    int check = 0;
    for (auto it = oldContributorList.begin(); it != oldContributorList.end(); it++)
    {
        oldConList << it->first << "," << it->second << std::endl;
        check += it->second;
    }
    assert(oldContributors.size() == check);
    decomListCum.close();
    oldConList.close();
    fout.close();
    decomList.close();
}

//Add the storage server ports into the files
void portHandler(json resultTx)
{
    // std::cout <<"Total master-nodes : "  << resultTx["result"]["master_node_states"][0] << std::endl;
    std::map<int, int> cumPortList;

    std::ofstream portList;
    portList.open("portList.csv"); //     fout.open("portList.csv", std::ios::app);

    std::ofstream listPort;
    listPort.open("cumPortList.csv"); //     fout.open("portList.csv", std::ios::app);
    for (auto master_node_data : resultTx["result"]["master_node_states"])
    {
        portList << master_node_data["master_node_pubkey"] << "," << master_node_data["public_ip"] << "," << master_node_data["storage_port"] << "," << master_node_data["storage_lmq_port"] << std::endl;
        ++cumPortList[master_node_data["storage_port"]];
    }

    int check = 0;
    for (auto it = cumPortList.begin(); it != cumPortList.end(); it++)
    {
        listPort << it->first << "," << it->second << std::endl;
        check += it->second;
    }
    assert(resultTx["result"]["master_node_states"].size() == check);
    portList.close();
    listPort.close();
}

void oxenportfetch()
{
    json transferBody = {
        {"jsonrpc", "2.0"},
        {"id", "0"},
        {"method", "get_service_nodes"},
        // {"params", {{"fields",{{"master_node_pubkey",true},{"public_ip",true},{"storage_port",true},{"storage_lmq_port",true}}}}}};
        {"params", {}}};

    // std::cout << "json formot : " << transferBody.dump() << std::endl;
    cpr::Response res = cpr::Post(cpr::Url{"http://public-na.optf.ngo:22023/json_rpc"},
                                  cpr::Body{transferBody.dump()},
                                  cpr::Header{{"Content-Type", "application/json"}});

    json resultTx = json::parse(res.text);

    std::map<int, int> cumPortList;

    std::ofstream portList;
    portList.open("oxenPortList.csv"); //     fout.open("portList.csv", std::ios::app);

    std::ofstream listPort;
    listPort.open("oxenCumPortList.csv"); //     fout.open("portList.csv", std::ios::app);

    std::cout << "Total service-nodes : " << resultTx["result"]["service_node_states"].size() << std::endl;

    for (auto service_node_data : resultTx["result"]["service_node_states"])
    {
        portList << service_node_data["service_node_pubkey"] << "," << service_node_data["public_ip"] << "," << service_node_data["storage_port"] << "," << service_node_data["storage_lmq_port"] << std::endl;
        ++cumPortList[service_node_data["storage_port"]];
    }

    int check = 0;
    for (auto it = cumPortList.begin(); it != cumPortList.end(); it++)
    {
        listPort << it->first << "," << it->second << std::endl;
        check += it->second;
    }
    assert(resultTx["result"]["service_node_states"].size() == check);
    portList.close();
    listPort.close();
}

// fetch the pubkeys from the ips which is listed in the ips.csv file
void ipToPubkey(json _resultTx)
{
    json mnData;
    std::ifstream inputfile("ips.csv");

    if(!inputfile.is_open()){
        std::cerr << "unable to open the input file" << std::endl;
        return;
    }
    // std::vector<std::string> ips = {"13.49.83.18","13.52.233.234","15.152.120.110"};
    std::vector<std::string> ips;
    std::string line;

    while (std::getline(inputfile, line)){
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ',')) {
            ips.push_back(token);
            std::cout << token << std::endl;
        }
    }

    for (auto master_node_data : _resultTx["result"]["master_node_states"])
    {
        auto it = std::find(ips.begin(),ips.end(),master_node_data["public_ip"]);
        if (it!= ips.end()){
            mnData.push_back(master_node_data);
        }
    }
    // std::cout << mnData << std::endl;
}

int main()
{
    json transferBody = {
        {"jsonrpc", "2.0"},
        {"id", "0"},
        {"method", "get_master_nodes"},
        // {"params", {{"fields",{{"master_node_pubkey",true},{"public_ip",true},{"storage_port",true},{"storage_lmq_port",true}}}}}};
        {"params", {}}};

    // std::cout << "json formot : " << transferBody.dump() << std::endl;
    cpr::Response res = cpr::Post(cpr::Url{"http://publicnode5.rpcnode.stream:29095/json_rpc"},
                                  cpr::Body{transferBody.dump()},
                                  cpr::Header{{"Content-Type", "application/json"}});

    json resultTx = json::parse(res.text);
    std::cout << "-------Data Received----------" << std::endl;
    masterNodeversionMonitor(resultTx);
    portHandler(resultTx);
    oxenportfetch();
    ipToPubkey(resultTx);
    return 0;
}
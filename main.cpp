#include <iostream>
#include <fstream>
#include "cpr/cpr.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <map>

using json = nlohmann::json;
using namespace std::chrono;

json belnetVersion = {0,9,7};
json storageVersion = {2,3,0};
json daemonVersion = {5,0,0};

int belnet0_9_7 = 0;
int daemon5_0_0 = 0;
int storage2_3_0 = 0;

std::map <std::string,int> contributorList;
std::vector <std::string> oldContributors;

void checkBelnetVersion(json belnetV, json belnetAV){
    for (int i =0;i<3;i++)
    {
        if(belnetV[i] == belnetAV[i]){
            continue;
        }
        else{
            return;
        }
    }
    belnet0_9_7 ++;
}

void checkDaemonVersion(json mnData,json daemonV, json daemonAV,std::ofstream &_fout){
    for (int i =0;i<3;i++)
    {
        if(daemonV[i] == daemonAV[i]){
            continue;
        }
        else{
            _fout << mnData["public_ip"] <<"," << mnData["master_node_pubkey"] <<"," << mnData["operator_address"] << std::endl;
            oldContributors.push_back(mnData["operator_address"]);
            ++contributorList[mnData["operator_address"]];
            return;
        }
    }
    daemon5_0_0 ++;
}
void checkStorageVersion(json storageV, json storageAV){
    for (int i =0;i<3;i++)
    {
        if(storageV[i] == storageAV[i]){
            continue;
        }
        else{
            return;
        }
    }
    storage2_3_0 ++;
}

int main()
{
    std::ofstream fout;
    fout.open("olddaemns.csv"); //     fout.open("olddaemns.csv", std::ios::app);

    std::ofstream conList;
    conList.open("contributorList.csv");

    json decomissionNodes;
    json transferBody = {
        {"jsonrpc", "2.0"},
        {"id", "0"},
        {"method", "get_master_nodes"},
        {"params", {}}};

    cpr::Response res = cpr::Post(cpr::Url{"http://explorer.beldex.io:19091/json_rpc"},
                                  cpr::Body{transferBody.dump()},
                                  cpr::Header{{"Content-Type", "application/json"}});

    json resultTx = json::parse(res.text);
    std::cout <<"Total master-nodes : "  << resultTx["result"]["master_node_states"].size() << std::endl;
    for(auto master_node_data : resultTx["result"]["master_node_states"]){
        checkBelnetVersion(master_node_data["belnet_version"],belnetVersion);
        checkDaemonVersion(master_node_data,master_node_data["master_node_version"],daemonVersion,fout);
        checkStorageVersion(master_node_data["storage_server_version"],storageVersion);
        if(!master_node_data["active"])
        {
            decomissionNodes.push_back(master_node_data);
        }
    }
    std::cout << "daemon5_0_0  : " << daemon5_0_0 << std::endl;
    std::cout << "storage2_3_0 : " << storage2_3_0 << std::endl;
    std::cout << "belnet0_9_7  : " << belnet0_9_7 << std::endl;
    std::cout << "decomissionNodes.size() : " << decomissionNodes.size() << std::endl;
    for(auto address : decomissionNodes)
        std::cout << "decomissionNodes address : " << address["operator_address"] << " ,IP : " << address["public_ip"] <<" and mn-key : " << address["pubkey_ed25519"]<< std::endl;

    assert((oldContributors.size() + daemon5_0_0) == resultTx["result"]["master_node_states"].size());

    int check =0;
    for(auto it=contributorList.begin(); it != contributorList.end(); it++)
    {
        conList << it->first << "," << it->second << std::endl;
        check+=it->second;
    }
    assert(oldContributors.size() == check);
    return 0;
}
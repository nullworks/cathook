/*
 * utfccp_commands.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: nullifiedcat
 */

#include "ucccccp.hpp"
#include "common.hpp"
#include "ucccccp_cmds.hpp"

CatCommand utfccp_encrypt(
    "ucccccp_encrypt", "Encrypt a message", [](const CCommand &args) {
        logging::Info("%s", ucccccp::encrypt(std::string(args.ArgS())).c_str());
    });

CatCommand utfccp_decrypt(
    "ucccccp_decrypt", "Decrypt a message", [](const CCommand &args) {
        if (ucccccp::validate(std::string(args.ArgS())))
        {
            logging::Info("%s",
                          ucccccp::decrypt(std::string(args.ArgS())).c_str());
        }
        else
        {
            logging::Info("Invalid input data!");
        }
    });

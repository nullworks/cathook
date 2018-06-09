/*
 * utfccp_commands.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: nullifiedcat
 */

#include "ucccccp.hpp"
#include "common.hpp"

CatCommand utfccp_encrypt(
    XORSTR("ucccccp_encrypt"), XORSTR("Encrypt a message"), [](const CCommand &args) {
        logging::Info(XORSTR("%s"), ucccccp::encrypt(std::string(args.ArgS())).c_str());
    });

CatCommand utfccp_decrypt(
    XORSTR("ucccccp_decrypt"), XORSTR("Decrypt a message"), [](const CCommand &args) {
        if (ucccccp::validate(std::string(args.ArgS())))
        {
            logging::Info(XORSTR("%s"),
                          ucccccp::decrypt(std::string(args.ArgS())).c_str());
        }
        else
        {
            logging::Info(XORSTR("Invalid input data!"));
        }
    });

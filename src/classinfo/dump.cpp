/*
 * dump.cpp
 *
 *  Created on: May 13, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

void PerformClassDump()
{
    ClientClass *cc = g_IBaseClient->GetAllClasses();
    FILE *cd        = fopen(XORSTR("/tmp/cathook-classdump.txt"), XORSTR("w"));
    if (cd)
    {
        while (cc)
        {
            fprintf(cd, XORSTR("[%d] %s\n"), cc->m_ClassID, cc->GetName());
            cc = cc->m_pNext;
        }
        fclose(cd);
    }
}

static CatCommand do_dump(XORSTR("debug_dump_classes"), XORSTR("Dump classes"),
                          PerformClassDump);

static CatCommand populate_dynamic(XORSTR("debug_populate_dynamic"),
                                   XORSTR("Populate dynamic class table"), []() {
                                       client_classes::dynamic_list.Populate();
                                   });

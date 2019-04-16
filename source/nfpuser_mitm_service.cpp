/*
 * Copyright (c) 2018 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <mutex>
#include <switch.h>
#include "nfpuser_mitm_service.hpp"
#include "nfp_shim.h"

extern FILE *g_logging_file;

void NfpUserMitmService::PostProcess(IMitmServiceObject *obj, IpcResponseContext *ctx) {
    /* Do nothing. */
}

Result NfpUserMitmService::CreateUserInterface(Out<std::shared_ptr<NfpUserInterface>> out) {
    fprintf(g_logging_file, "NfpUserMitmService::CreateUserInterface()\n");
    fflush(g_logging_file);
    
    std::shared_ptr<NfpUserInterface> intf = nullptr;
    u32 out_domain_id = 0;
    Result rc = 0;
    ON_SCOPE_EXIT {
        if (R_SUCCEEDED(rc)) {
            out.SetValue(std::move(intf));
            if (out.IsDomain()) {
                out.ChangeObjectId(out_domain_id);
                fprintf(g_logging_file, "Is domain with objid %u\n", out.GetObjectId());
                fflush(g_logging_file);
            }
        }
    };
    
    NfpUser user;
    rc = nfpCreateUserInterface(this->forward_service.get(), &user);
    if (R_SUCCEEDED(rc)) {
        intf = std::make_shared<NfpUserInterface>(new NfpUser(user));
        if (out.IsDomain()) {
            out_domain_id = user.s.object_id;
        }
    }

    return rc;
}

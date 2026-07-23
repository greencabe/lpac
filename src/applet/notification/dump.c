#include "notification_common.h"
#include "notification_sequence.h"
#include "process.h"

#include <euicc/es10b.h>
#include <euicc/es10c.h>
#include <lpac/utils.h>

#include <main.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static bool retrieve_notification(const char *eid, const uint32_t seqNumber) {
    _cleanup_(es10b_pending_notification_free) struct es10b_pending_notification notification;

    if (es10b_retrieve_notifications_list(&euicc_ctx, &notification, seqNumber)) {
        jprint_error("es10b_retrieve_notifications_list", NULL);
        return false;
    }

    _cleanup_cjson_ cJSON *jroot = build_notification(eid, seqNumber, &notification);
    if (jroot == NULL)
        return false;

    _cleanup_cjson_free_ char *jstr = cJSON_PrintUnformatted(jroot);
    printf("%s\n", jstr);
    fflush(stdout);

    return true;
}

static int applet_main(const int argc, char **argv) {
    static const char *opt_string = "ah?";

    int fret = 0;
    int all = 0;
    int opt = 0;
    char *eid = NULL;

    while ((opt = getopt(argc, argv, opt_string)) != -1) {
        switch (opt) {
        case 'a':
            all = 1;
            break;
        case 'h':
        case '?':
            printf("Usage: %s [OPTIONS] [seqNumber_0] [seqNumber_1]...\n", argv[0]);
            printf("\t -a All notifications\n");
            return -1;
        default:
            break;
        }
    }

    if (es10c_get_eid(&euicc_ctx, &eid)) {
        jprint_error("es10c_get_eid", NULL);
        return -1;
    }

    if (all) {
        _cleanup_es10b_notification_metadata_list_ struct es10b_notification_metadata_list *notifications, *rptr;

        if (es10b_list_notification(&euicc_ctx, &notifications)) {
            jprint_error("es10b_list_notification", NULL);
            return -1;
        }

        rptr = notifications;
        while (rptr) {
            if (!retrieve_notification(eid, (uint32_t)rptr->seqNumber)) {
                fret = -1;
                break;
            }
            rptr = rptr->next;
        }

    } else {
        for (int i = optind; i < argc; i++) {
            uint32_t seqNumber;
            if (!notification_parse_sequence_number(argv[i], &seqNumber)) {
                jprint_error("seqNumber", "invalid unsigned 32-bit decimal");
                return -1;
            }
            if (!retrieve_notification(eid, seqNumber)) {
                fret = -1;
                break;
            }
        }
    }

    return fret;
}

struct applet_entry applet_notification_dump = {
    .name = "dump",
    .main = applet_main,
};

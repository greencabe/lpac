#include "remove.h"
#include "notification_sequence.h"

#include <euicc/es10b.h>
#include <lpac/utils.h>

#include <main.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int _delete_single(uint32_t seqNumber) {
    char str_seqNumber[11];
    int ret;

    snprintf(str_seqNumber, sizeof(str_seqNumber), "%u", seqNumber);

    jprint_progress("es10b_remove_notification_from_list", str_seqNumber);
    if ((ret = es10b_remove_notification_from_list(&euicc_ctx, seqNumber))) {
        const char *reason;
        switch (ret) {
        case 1:
            reason = "seqNumber not found";
            break;
        default:
            reason = "unknown";
            break;
        }
        jprint_error("es10b_remove_notification_from_list", reason);
        return -1;
    }

    return 0;
}

static int applet_main(int argc, char **argv) {
    static const char *opt_string = "ah?";

    int fret = 0;
    int all = 0;
    int opt = 0;

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

    if (all) {
        _cleanup_es10b_notification_metadata_list_ struct es10b_notification_metadata_list *notifications, *rptr;

        jprint_progress("es10b_list_notification", NULL);
        if (es10b_list_notification(&euicc_ctx, &notifications)) {
            jprint_error("es10b_list_notification", NULL);
            return -1;
        }

        rptr = notifications;
        while (rptr) {
            if (_delete_single((uint32_t)rptr->seqNumber)) {
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

            if (_delete_single(seqNumber)) {
                fret = -1;
                break;
            }
        }
    }

    if (fret == 0) {
        jprint_success(NULL);
    }

    return fret;
}

struct applet_entry applet_notification_remove = {
    .name = "remove",
    .main = applet_main,
};

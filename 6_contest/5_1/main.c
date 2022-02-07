#include <sys/stat.h>

struct Task
{
    unsigned uid;
    int gid_count;
    unsigned *gids;
};

enum { MAX_RIGHTS = 7 };

enum UserType { ROOT, OWNER, GROUP, OTHERS };
enum UserToBias { BIAS_OWNER = 6, BIAS_GROUP = 3, BIAS_OTHERS = 0 };

static enum UserType
get_user_type(const struct stat *stb, const struct Task *task)
{
    enum UserType user_type;
    if (task->uid == 0) {
        user_type = ROOT;
    } else if (task->uid == stb->st_uid) {
        user_type = OWNER;
    } else {
        user_type = OTHERS;
        for (int i = 0; user_type != GROUP && i < task->gid_count; i++) {
            if (task->gids[i] == stb->st_gid) {
                user_type = GROUP;
            }
        }
    }
    return user_type;
}

static int
get_user_rights(const struct stat *stb, enum UserType user_type)
{
    if (user_type == ROOT) {
        return MAX_RIGHTS;
    } else {
        int bias;
        if (user_type == OWNER) {
            bias = BIAS_OWNER;
        } else if (user_type == GROUP) {
            bias = BIAS_GROUP;
        } else {
            bias = BIAS_OTHERS;
        }
        return (stb->st_mode >> bias) & MAX_RIGHTS;
    }
}

int
myaccess(const struct stat *stb, const struct Task *task, int access)
{
    // aux types
    enum UserType user_type = get_user_type(stb, task);
    int rights = get_user_rights(stb, user_type);
    return (access & rights) == access;
}

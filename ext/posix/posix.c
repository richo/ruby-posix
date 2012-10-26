#include <ruby.h>
#include <ruby/st.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

void Init_posix(void);
static VALUE rb_hash_keys(VALUE hash);

#define RB_POSIX_SIG_BLOCK SIG_BLOCK
#define RB_POSIX_SIG_UNBLOCK SIG_UNBLOCK
#define RB_POSIX_SIG_SETMASK SIG_SETMASK

#define RB_POSIX_O_RDONLY O_RDONLY
#define RB_POSIX_O_WRONLY O_WRONLY
#define RB_POSIX_O_RDWR O_RDWR
#define RB_POSIX_O_NONBLOCK O_NONBLOCK
#define RB_POSIX_O_APPEND O_APPEND
#define RB_POSIX_O_CREAT O_CREAT
#define RB_POSIX_O_TRUNC O_TRUNC
#define RB_POSIX_O_EXCL O_EXCL
#define RB_POSIX_O_SHLOCK O_SHLOCK
#define RB_POSIX_O_EXLOCK O_EXLOCK
#define RB_POSIX_O_NOFOLLOW O_NOFOLLOW
#define RB_POSIX_O_SYMLINK O_SYMLINK
#define RB_POSIX_O_EVTONLY O_EVTONLY
#define RB_POSIX_O_CLOEXEC O_CLOEXEC

int rb_Sigset2sigset_t(VALUE rb_Sigset, sigset_t *sigset) {
    int i;
    VALUE signals;


    sigemptyset(sigset);
    signals = rb_iv_get(rb_Sigset, "@signals");

    for (i = 0; i < RARRAY_LEN(signals); i++) {
        sigaddset(sigset, FIX2INT(RARRAY_PTR(signals)[i]));
    }
    return 0;
}

/* @see man 2 sigprocmask
 *
 * Doesn't accept a third argument, instead returns the new set
 */

VALUE posix_sigprocmask(VALUE self, VALUE _how, VALUE _set) {
    int how;
    sigset_t *set;
    sigset_t *oset = NULL; // TODO, storage for returnvalue

    how = FIX2INT(_how);

    if (_set == Qnil) {
        set = NULL;
    } else {
        set = malloc(sizeof(sigset_t));
        rb_Sigset2sigset_t(_set, set);
    }

    sigemptyset(set);
    sigaddset(set, SIGINT);

    if (sigprocmask(how, set, oset) == 0) {
        // TODO Construct a Sigset and return it so that we can interrogate it

        if(set)
            free(set);
        return Qtrue;
    } else {
        if(set)
            free(set);
        rb_raise(rb_eException, "%s", strerror(errno));
    }
}

/* Export an argv that does sane, reasonable things instead of doing weird
 * shit.
 *
 * Return value is largely irrelevant
 */
VALUE posix_execve(VALUE self, VALUE _binary, VALUE _argv, VALUE _envp) {
    /* Iterators */
    int i;
    VALUE akey, keys, envk;

    /* Binary to load in the new process */
    char* binary = StringValueCStr(_binary);

    /* Construct our new process' argv. Onus is on
     * the programmer to set ARGV[0] to something
     * reasonable. */
    char** argv = malloc(sizeof(char*)*RARRAY_LEN(_argv) + 1);

    for (i = 0; i < RARRAY_LEN(_argv); i++) {
        argv[i] = StringValuePtr(RARRAY_PTR(_argv)[i]);
        argv[i+1] = NULL; /* Ensure that we're null terminated */
    }

    /* Construct our environment. Note that this totally ignores the precedent
     * set by Process#spawn, Kernel#exec and fiends */

    char **envp = malloc(sizeof(char**)*RHASH(_envp)->ntbl->num_entries + 1);

    keys = rb_hash_keys(_envp);
    for (i = 0; i < RARRAY_LEN(keys); i++) {
        akey = RARRAY_PTR(keys)[i];
        envk = rb_hash_aref(_envp, akey);
        asprintf(&envp[i], "%s=%s", StringValuePtr(akey), StringValuePtr(envk));
        envp[i+1] = NULL; /* Ensure that we're null terminated */
    }

    execve(binary, argv, envp);
    fprintf(stderr, "Error: %s", strerror(errno));
}

VALUE posix_open(VALUE self, VALUE _path, VALUE _mode) {


void Init_posix(void) {
    /* Create ::Posix */
    VALUE klass = rb_define_class("Posix", rb_cObject);

    /* Define constants for sigprocmask */
    rb_define_const(klass, "SIG_BLOCK", INT2FIX(RB_POSIX_SIG_BLOCK));
    rb_define_const(klass, "SIG_UNBLOCK", INT2FIX(RB_POSIX_SIG_UNBLOCK));
    rb_define_const(klass, "SIG_SETMASK", INT2FIX(RB_POSIX_SIG_SETMASK));

    /* Method binding for sigprocmask */
    rb_define_singleton_method(klass, "sigprocmask", posix_sigprocmask, 2);
    rb_define_singleton_method(klass, "execve", posix_execve, 3);

    /* Define constants for open */
    rb_define_const(klass, "O_RDONLY", INT2FIX(O_RDONLY));
    rb_define_const(klass, "O_WRONLY", INT2FIX(O_WRONLY));
    rb_define_const(klass, "O_RDWR", INT2FIX(O_RDWR));
    rb_define_const(klass, "O_NONBLOCK", INT2FIX(O_NONBLOCK));
    rb_define_const(klass, "O_APPEND", INT2FIX(O_APPEND));
    rb_define_const(klass, "O_CREAT", INT2FIX(O_CREAT));
    rb_define_const(klass, "O_TRUNC", INT2FIX(O_TRUNC));
    rb_define_const(klass, "O_EXCL", INT2FIX(O_EXCL));
    rb_define_const(klass, "O_SHLOCK", INT2FIX(O_SHLOCK));
    rb_define_const(klass, "O_EXLOCK", INT2FIX(O_EXLOCK));
    rb_define_const(klass, "O_NOFOLLOW", INT2FIX(O_NOFOLLOW));
    rb_define_const(klass, "O_SYMLINK", INT2FIX(O_SYMLINK));
    rb_define_const(klass, "O_EVTONLY", INT2FIX(O_EVTONLY));
    rb_define_const(klass, "O_CLOEXEC", INT2FIX(O_CLOEXEC));

    /* Method binding for open */
    rb_define_singleton_method(klass, "open", posix_open, 2);
}


// Shamelessly stolen from hash.c
static int
keys_i(VALUE key, VALUE value, VALUE ary)
{
        rb_ary_push(ary, key);
        return ST_CONTINUE;
}

static VALUE
rb_hash_keys(VALUE hash)
{
    VALUE ary;

    ary = rb_ary_new();
    rb_hash_foreach(hash, keys_i, ary);

    return ary;
}

#include <ruby.h>
#include <ruby/st.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

void Init_posix(void);
static VALUE rb_hash_keys(VALUE hash);

#define RB_POSIX_SIG_BLOCK SIG_BLOCK
#define RB_POSIX_SIG_UNBLOCK SIG_UNBLOCK
#define RB_POSIX_SIG_SETMASK SIG_SETMASK

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

VALUE posix_dup(VALUE self, VALUE _filedes) {
    int filedes = FIX2INT(_filedes);

    return INT2FIX(dup(filedes));
}

VALUE posix_dup2(VALUE self, VALUE _filedes, VALUE _filedes2) {
    int filedes = FIX2INT(_filedes);
    int filedes2 = FIX2INT(_filedes2);

    return INT2FIX(dup2(filedes, filedes2));
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
    argv[0] = NULL;

    for (i = 0; i < RARRAY_LEN(_argv); i++) {
        argv[i] = StringValuePtr(RARRAY_PTR(_argv)[i]);
        argv[i+1] = NULL; /* Ensure that we're null terminated */
    }

    /* Construct our environment. Note that this totally ignores the precedent
     * set by Process#spawn, Kernel#exec and fiends */

    char **envp;
    if (RHASH(_envp)->ntbl) {
        envp = malloc(sizeof(char**)*RHASH(_envp)->ntbl->num_entries + 1);
        keys = rb_hash_keys(_envp);
        for (i = 0; i < RARRAY_LEN(keys); i++) {
            akey = RARRAY_PTR(keys)[i];
            envk = rb_hash_aref(_envp, akey);
            asprintf(&envp[i], "%s=%s", StringValuePtr(akey), StringValuePtr(envk));
            envp[i+1] = NULL; /* Ensure that we're null terminated */
    }
    } else {
        envp = malloc(sizeof(char**));
        envp[0] = NULL;
    }

    execve(binary, argv, envp);
    fprintf(stderr, "Error: %s", strerror(errno));
}

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

    /* Method binding for dup */
    rb_define_singleton_method(klass, "dup", posix_dup, 1);
    rb_define_singleton_method(klass, "dup2", posix_dup2, 2);
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

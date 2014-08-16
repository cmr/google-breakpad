#include "client/linux/handler/exception_handler.h"
#include <stdio.h>
#include <stdlib.h>

using namespace google_breakpad;

typedef ExceptionHandler::MinidumpCallback MinidumpCallback;
typedef ExceptionHandler::FilterCallback FilterCallback;

struct WrapperContext {
    MinidumpCallback mcb;
    FilterCallback fcb;
    void *real_context;

    WrapperContext(FilterCallback fcb, MinidumpCallback mcb, void *context);
};

WrapperContext::WrapperContext(FilterCallback fcb, MinidumpCallback mcb, void *context) {
    this->mcb = mcb;
    this->fcb = fcb;
    this->real_context = context;
}

static bool filter_callback_wrapper(void *context) {
    WrapperContext *cont = reinterpret_cast<WrapperContext*>(context);

    if (cont->fcb) {
        return cont->fcb(cont->real_context);
    } else {
        return true;
    }
}

static bool minidump_callback_wrapper(const MinidumpDescriptor &desc, void *context, bool succeeded) {
    WrapperContext *cont = reinterpret_cast<WrapperContext*>(context);

    if (cont->mcb) {
        return cont->mcb(desc, cont->real_context, succeeded);
    } else {
        return succeeded;
    }
}

extern "C" {
    void *rust_breakpad_descriptor_new(const char *path) {
        return reinterpret_cast<void*>(new MinidumpDescriptor(path));
    }

    const char *rust_breakpad_descriptor_path(void *desc_) {
        return reinterpret_cast<MinidumpDescriptor*>(desc_)->path();
    }

    void rust_breakpad_descriptor_free(void *desc) {
        delete reinterpret_cast<MinidumpDescriptor*>(desc);
    }

    void *rust_breakpad_exceptionhandler_new(void *desc, FilterCallback fcb,
            MinidumpCallback mcb, void *context, int install_handler) {

        WrapperContext *wrapper_context = new WrapperContext(fcb, mcb, context);

        ExceptionHandler *eh = new ExceptionHandler(
            *reinterpret_cast<MinidumpDescriptor*>(desc),
            filter_callback_wrapper,
            minidump_callback_wrapper,
            reinterpret_cast<void*>(wrapper_context),
            install_handler,
            -1);

        return reinterpret_cast<void*>(eh);
    }

    int rust_breakpad_exceptionhandler_write_minidump(void *eh) {
        return reinterpret_cast<ExceptionHandler*>(eh)->WriteMinidump();
    }

    void rust_breakpad_exceptionhandler_free(void *eh) {
        delete reinterpret_cast<ExceptionHandler*>(eh);
    }
}

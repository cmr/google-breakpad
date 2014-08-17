typedef void(__cdecl *_purecall_handler)(void);
#include "../windows/handler/exception_handler.h"

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

static bool filter_callback_wrapper(void *context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion) {
	WrapperContext *cont = reinterpret_cast<WrapperContext*>(context);

	return cont->fcb ? cont->fcb(cont->real_context, exinfo, assertion) : true;
}

static bool minidump_callback_wrapper(const wchar_t *dump_path, const wchar_t *minidump_id, void *context, EXCEPTION_POINTERS *exinfo, MDRawAssertionInfo *assertion, bool succeeded) {
	WrapperContext *cont = reinterpret_cast<WrapperContext*>(context);

	return cont->mcb ? cont->mcb(dump_path, minidump_id, cont->real_context, exinfo, assertion, succeeded) : succeeded;
}

extern "C" {
	void *rust_breakpad_exceptionhandler_new(const char *desc, FilterCallback fcb,
		MinidumpCallback mcb, void *context, int install_handler) {

		WrapperContext *wrapper_context = new WrapperContext(fcb, mcb, context);

		ExceptionHandler *eh = new ExceptionHandler(
			wstring(reinterpret_cast<const wchar_t*>(desc)),
			filter_callback_wrapper,
			minidump_callback_wrapper,
			reinterpret_cast<void*>(wrapper_context),
			install_handler ? ExceptionHandler::HANDLER_ALL : ExceptionHandler::HANDLER_NONE
			);

		return reinterpret_cast<void*>(eh);
	}

	int rust_breakpad_exceptionhandler_write_minidump(void *eh) {
		return reinterpret_cast<ExceptionHandler*>(eh)->WriteMinidump();
	}

	void rust_breakpad_exceptionhandler_free(void *eh) {
		delete reinterpret_cast<ExceptionHandler*>(eh);
	}
}

#include "common.h"
#include "error.h"
#include "logging.h"

Error* create_error(Allocator *alloc, int code, const char *reason) {
    DEBUG(alloc->logger, "create_error(alloc=%p, code=%d, reason=%s)", alloc, code, reason);
    Error *err = (Error*)allocator_alloc(alloc, sizeof(Error));
    err->code = code;
    err->reason = string_dup_cstr(alloc, reason);
    return err;
}

String* error_repr(Error *error, Allocator *alloc) {
    return string_sprintf(alloc, "Error{code=%d, reason='%.*s'}", 
        error->code, error->reason->length, error->reason->data);
}
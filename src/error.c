#include "error.h"
#include "common.h"
#include "logging.h"

Error *create_error(Allocator *alloc, int code, const char *reason, Error *from) {
    DEBUG(alloc->logger, "create_error(alloc=%p, code=%d, reason=%s)", alloc, code, reason);
    Error *err = (Error *)allocator_alloc(alloc, sizeof(Error));
    err->from = from;
    err->code = code;
    err->reason = string_dup_cstr(alloc, reason);
    return err;
}

String *error_repr(Error *error, Allocator *alloc) {
    // TODO: include Error "from" in repr
    return string_sprintf(alloc, "Error{code=%d, reason='%.*s'}", error->code,
                          error->reason->length, error->reason->data);
}
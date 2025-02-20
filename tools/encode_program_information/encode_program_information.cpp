// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include "ebpf_api.h"
#include "ebpf_nethooks.h"
#include "ebpf_platform.h"
#include "ebpf_program_types.h"
#include "ebpf_windows.h"

static ebpf_error_code_t
_emit_program_information_file(
    const char* file_name, const char* symbol_name, uint8_t* buffer, unsigned long buffer_size)
{
    unsigned long index;
    FILE* output;
    if (fopen_s(&output, file_name, "w") != 0)
        return EBPF_ERROR_OUT_OF_RESOURCES;

    fprintf(output, "#pragma once\n");
    fprintf(output, "#include <stdint.h>\n");
    fprintf(output, "static const uint8_t %s[] = {", symbol_name);
    for (index = 0; index < buffer_size; index++) {
        if (index % 16 == 0)
            fprintf(output, "\n");

        fprintf(output, "0x%.2X, ", buffer[index]);
    }
    fprintf(output, "};");
    fflush(output);
    fclose(output);
    return EBPF_ERROR_SUCCESS;
}

static ebpf_error_code_t
_encode_bind()
{
    ebpf_error_code_t return_value;
    uint8_t* buffer = NULL;
    unsigned long buffer_size = 0;
    ebpf_context_descriptor_t bind_context_descriptor = {
        sizeof(bind_md_t), EBPF_OFFSET_OF(bind_md_t, app_id_start), EBPF_OFFSET_OF(bind_md_t, app_id_end), -1};
    ebpf_program_type_descriptor_t bind_program_type = {"bind", &bind_context_descriptor, EBPF_PROGRAM_TYPE_BIND};
    ebpf_program_information_t bind_program_information = {bind_program_type, 0, NULL};

    return_value = ebpf_program_information_encode(&bind_program_information, &buffer, &buffer_size);
    if (return_value != EBPF_ERROR_SUCCESS)
        goto Done;

    return_value = _emit_program_information_file(
        "ebpf_bind_program_data.h", "_ebpf_encoded_bind_program_information_data", buffer, buffer_size);
    if (return_value != EBPF_ERROR_SUCCESS)
        goto Done;

    return_value = EBPF_ERROR_SUCCESS;

Done:
    ebpf_free(buffer);

    return return_value;
}

static ebpf_error_code_t
_encode_xdp()
{
    ebpf_error_code_t return_value;
    uint8_t* buffer = NULL;
    unsigned long buffer_size = 0;
    ebpf_context_descriptor_t xdp_context_descriptor = {
        sizeof(xdp_md_t),
        EBPF_OFFSET_OF(xdp_md_t, data),
        EBPF_OFFSET_OF(xdp_md_t, data_end),
        EBPF_OFFSET_OF(xdp_md_t, data_meta)};
    ebpf_program_type_descriptor_t xdp_program_type = {"xdp", &xdp_context_descriptor, EBPF_PROGRAM_TYPE_XDP};
    ebpf_program_information_t xdp_program_information = {xdp_program_type, 0, NULL};

    return_value = ebpf_program_information_encode(&xdp_program_information, &buffer, &buffer_size);
    if (return_value != EBPF_ERROR_SUCCESS)
        goto Done;

    return_value = _emit_program_information_file(
        "ebpf_xdp_program_data.h", "_ebpf_encoded_xdp_program_information_data", buffer, buffer_size);
    if (return_value != EBPF_ERROR_SUCCESS)
        goto Done;

    return_value = EBPF_ERROR_SUCCESS;

Done:
    ebpf_free(buffer);

    return return_value;
}

int
main()
{
    if (_encode_xdp() != EBPF_ERROR_SUCCESS || _encode_bind() != EBPF_ERROR_SUCCESS)
        return 1;
    return 0;
}

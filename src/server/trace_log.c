/**
 * *** Copyright Notice ***
 * SDS - Scientific Data Services framework, Copyright (c) 2015, The Regents of the University of California, through Lawrence Berkeley National Laboratory (subject to receipt of any required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * If you have questions about your rights to use or distribute this software,
 * please contact Berkeley Lab's Technology Transfer Department at TTD@lbl.gov.
 *
 * NOTICE.  This software was developed under funding from the
 * U.S. Department of Energy.  As such, the U.S. Government has been granted
 * for itself and others acting on its behalf a paid-up, nonexclusive,
 * irrevocable, worldwide license in the Software to reproduce, prepare
 * derivative works, and perform publicly and display publicly.
 * Beginning five (5) years after the date permission to assert copyright is
 * obtained from the U.S. Department of Energy, and subject to any subsequent
 * five (5) year renewals, the U.S. Government is granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable, worldwide license
 * in the Software to reproduce, prepare derivative works, distribute copies to
 * the public, perform publicly and display publicly, and to permit others to
 * do so.
 *
*/

/**
 *
 * Email questions to {dbin, sbyna, kwu}@lbl.gov
 * Scientific Data Management Research Group
 * Lawrence Berkeley National Laboratory
 *
*/
/*
 * This file contains the functions to analyze the trace log
 * Author: Tang Houjun  and Bin Dong <dbin at lbl.gov >
 * Copyright 2015 the Regents of the University of California
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "sds-common.h"
#include "trace_log.h"

#include <sys/socket.h>
#include "sds-server.h"
#include "message.protoc.pb-c.h"

extern char         sds_root_path[MAX_FILE_NAME_LENGTH];
extern sds_dbs_t    g_sds_dbs;


// Print pattern with a good format
int print_pattern(RequestTraceData *trace_data, int nblock, FILE *fout)
{
    int i, j;
    int ndim = trace_data->ndim;

    // Print out data selection from server
    fprintf(fout, "%llu ", trace_data->time);
    if (trace_data->pattern_type == SDS_PAPPTERN_HYPERSLAB) {
        /* nblock = trace_data->n_h_pattern / ndim / 2; */
        fprintf(fout, "Hyperslab %d\n", nblock);
        for (i = 0; i < nblock; i++) {
            fprintf(fout, "(");
            for (j = 0; j < ndim; j++) {
                if (j != 0) fprintf(fout, ", ");
                fprintf(fout, "%llu", trace_data->h_pattern[i]->start[i]);
            }
            fprintf(fout, ") - (");
            for (j = 0; j < ndim; j++) {
                if (j != 0) fprintf(fout, ", ");
                fprintf(fout, "%llu", trace_data->h_pattern[i]->count[i]);
                // NOTE: count[] is actually the lower right corner
            }
            fprintf(fout, ")\n");
            /* fprintf(fout, "\n"); */
        }
    }
    else if (trace_data->pattern_type == SDS_PAPPTERN_ELEMENT) {
        /* nblock = trace_data->e_pattern->n_coordination; */
        fprintf(fout, "Element %d\n", nblock);
        for (i = 0; i < nblock; i++) {
            if (i % ndim == 0) {
                if (i != 0)
                    fprintf(fout, ")\n");
                fprintf(fout, "(");
            }
            else
                fprintf(fout, ", ");
            fprintf(fout, "%d", trace_data->e_pattern->coordination[i]);
        }
        fprintf(fout, ")\n");
    }

    return 0;
}

int print_layout_metadata(LayoutMetadataT* meta)
{
    FILE *fout = stderr;
    int i, j;
    int ndim   = meta->n_req_global_start;

    log_msg("metadata global end:");
    for (i = 0; i < ndim; i++) {
        log_msg("%d", meta->req_global_end[i]);
    }

    if (meta->pattern_type == SDS_PAPPTERN_HYPERSLAB) {
        int nblock = meta->n_req_hyperslab;
        fprintf(fout, "Hyperslab %d\n", nblock);
        for (i = 0; i < nblock; i++) {
            fprintf(fout, "(");
            for (j = 0; j < ndim; j++) {
                if (j != 0) fprintf(fout, ", ");
                fprintf(fout, "%llu", meta->req_hyperslab[i]->start[i]);
            }
            fprintf(fout, ") - (");
            for (j = 0; j < ndim; j++) {
                if (j != 0) fprintf(fout, ", ");
                fprintf(fout, "%llu", meta->req_hyperslab[i]->count[i]);
                // NOTE: count[] is actually the lower right corner
            }
            fprintf(fout, ")\n");
        }
    }
    fprintf(stderr, "Linear_order=%d\n", meta->linear_order);

    return 0;
}

// Tang
// Dump the trace data to file
// Output: $SDS_ROOT_DIR/trace/{path_to_data/}data_filename/group_name/dataset_name.mpi_rank
//         One file per process
int save_pattern_to_file(RequestTraceData *trace_data, int nblock)
{
    SdsObject         *sds_object;
    int                ndim;

    sds_object = trace_data->object;
    ndim = trace_data->ndim;

    // write the selection to file under $SDS_ROOT_DIR/trace/{path_to_data/}data_filename/group_name/dataset_name.time
    FILE * f_trace;
    char trace_path[MAX_FILE_NAME_LENGTH], trace_name[MAX_FILE_NAME_LENGTH];
    /* sprintf(trace_name, "%s/trace/%s/%s/%s/%s.%d", client_sds_root_path, d->dir_name, d->file_name, d->group_name, d->dataset_name, mpi_rank); */
    sprintf(trace_path, "%s/trace/%s/%s", sds_root_path, sds_object->filename, sds_object->group);
    int iter_path;
    // deal with cases using "./" in the path
    for (iter_path = 0; iter_path < strlen(trace_path) - 1; iter_path++) {
        if (trace_path[iter_path] == '.' && trace_path[iter_path+1] == '/') {
            trace_path[iter_path] = '/';
        }
    }
    if (trace_path[strlen(trace_path)-1] == '.')
        trace_path[strlen(trace_path)-1] = 0;

    log_msg("Pattern location: [%s]", trace_path);

    // make dir
    char *p_trace_path = NULL;
    for(p_trace_path = trace_path + 1; *p_trace_path; p_trace_path++) {
        if(*p_trace_path == '/') {
            while(*(p_trace_path+1) == '/')
                p_trace_path++;
            *p_trace_path = 0;
            mkdir(trace_path, S_IRWXU);
            *p_trace_path = '/';
        }
    }
    mkdir(trace_path, S_IRWXU);

    sprintf(trace_name, "%s/%s.%d", trace_path, sds_object->dsetname, trace_data->mpi_rank);

    f_trace = fopen (trace_name, "a+");
    if (f_trace == NULL) {
        log_msg("Cannot write to trace file [%s]", trace_name);
    }
    else {
        print_pattern(trace_data, nblock, f_trace);
    }
    fclose (f_trace);

    return 0;
}

ResponseTraceData* convert_layout_metadata_to_responsetracedata(LayoutMetadataT * layout, int nblock)
{
    int i;
    // Fill in the response structure
    ResponseTraceData * trace = malloc( sizeof(ResponseTraceData) );
    response_trace_data__init(trace);

    trace->opt_file        = layout->opt_file;
    trace->n_opt_hyperslab = layout->n_opt_hyperslab;

    trace->opt_hyperslab   = malloc(nblock * sizeof(H5Hyperslab*));
    for (i = 0; i < trace->n_opt_hyperslab; i++)
        trace->opt_hyperslab[i]  = layout->opt_hyperslab[i];
    return trace;
}

int insert_selection_as_layout_to_db(SDSMetadataDbKey *key, SDSMetadataDbValue *value, RequestTraceData *trace_data, int nblock, int ndim, uint64_t *global_start, uint64_t *global_end, SdsObject *sds_object)
{
    int i, ret;
    // Insert one layout for debug purposes
    LayoutMetadataT layout = LAYOUT_METADATA_T__INIT;
    if (value->n_layout_metadata == 0) {

        // Populate the layout metadata
        layout.pattern_type          = SDS_PAPPTERN_HYPERSLAB;

        layout.n_req_global_start    = ndim;
        layout.n_req_global_end      = ndim;
        layout.req_global_start      = global_start;
        layout.req_global_end        = global_end;

        layout.n_req_hyperslab       = nblock;
        layout.req_hyperslab         = malloc(nblock * sizeof(H5Hyperslab*));

        for (i = 0; i < nblock; i++) {
            /* layout.hyperslab[i]      = malloc(sizeof(H5Hyperslab)); */
            /* h5_hyperslab__init(layout.hyperslab[i]); */
            layout.req_hyperslab[i]  = (H5Hyperslab*)trace_data->h_pattern[i];
        }

        SdsFile opt_file = SDS_FILE__INIT;
        opt_file.filename = "./new.h5";
        /* opt_file.filename = sds_object->filename; */
        opt_file.group    = sds_object->group;
        opt_file.dsetname = sds_object->dsetname;
        opt_file.filetype = 1;
        opt_file.datatype = 1;
        opt_file.ir_type  = 1;
        layout.opt_file              = &opt_file;
        layout.opt_file_status       = FILE_STATUS__IN_SERVICE;
        log_msg("opt file: %s | %s | %s", layout.opt_file->filename, layout.opt_file->group, layout.opt_file->dsetname);

        layout.n_opt_hyperslab       = nblock;
        layout.opt_hyperslab         = malloc(nblock * sizeof(H5Hyperslab*));
        for (i = 0; i < nblock; i++) {
            layout.opt_hyperslab[i]  = (H5Hyperslab*)trace_data->h_pattern[i];
        }

        layout.linear_order      = 1;
        layout.n_chunk_size      = 0;
        layout.chunk_size        = NULL;

        value->n_layout_metadata   = 1;
        value->layout_metadata     = malloc(1 * sizeof(LayoutMetadataT*));
        value->layout_metadata[0]  = &layout;

        print_layout_metadata(value->layout_metadata[0]);

        // Write the record to DB
        /* log_msg("Start to write metadata record to DB..."); */
        ret = write_metadata_record(g_sds_dbs.sds_metadata_dbp, key, value);
        if (ret == 0) {
            log_msg("Write metadata record to DB...[SUCCESS]");
        }
        else {
            log_msg("Write metadata record to DB...[FAILED]");
        }
    }
    return 0;
}

ResponseTraceData * trace_log_analysis(client_t *client) {
    SdsObject         *sds_object;
    SDSMetadataDbKey   key = SDS_METADATA_DB_KEY__INIT;
    SDSMetadataDbValue value;
    int                mpi_size, mpi_rank, ptn_type, ndim, nblock, ret;
    int                i, j, k;
    char              *dir_name, *app_name;
    uint64_t           ptn_time;

    RequestTraceData *trace_data;
    trace_data = client->request->trace_data;
    sds_object = trace_data->object;

    //Figure out the key to store the data
    key.filename     = sds_object->filename;
    key.group        = sds_object->group;
    key.dsetname     = sds_object->dsetname;
    key.datatype     = sds_object->data_type;
    key.filetype     = sds_object->file_type;

    mpi_size         = trace_data->mpi_size;
    mpi_rank         = trace_data->mpi_rank;
    ptn_type         = trace_data->pattern_type;
    ptn_time         = trace_data->time;
    ndim             = trace_data->ndim;
    if (trace_data->pattern_type == SDS_PAPPTERN_HYPERSLAB)
        nblock = trace_data->n_h_pattern / ndim / 2;
    else if (trace_data->pattern_type == SDS_PAPPTERN_ELEMENT)
        nblock = trace_data->e_pattern->n_coordination;
    else {
        log_msg("trace_log_analysis(): unsupported pattern type");
        return NULL;
    }

    log_msg("==== Trace received (mpi_size: %d, mpi_rank: %d, pattern type: %d, time: %llu) !", mpi_size, mpi_rank, ptn_type, ptn_time);

    // Save pattern to file
    save_pattern_to_file(trace_data, nblock);

    // Print pattern
    log_msg("\n==== Received Pattern ====");
    print_pattern(trace_data, nblock, stderr);
    log_msg("==== END Pattern ====\n");

    // Calculate the byte start/end range of received pattern.
    uint64_t *global_start, *global_end;
    global_start = (uint64_t *)malloc(ndim * sizeof(uint64_t));
    global_end   = (uint64_t *)malloc(ndim * sizeof(uint64_t));

    if (trace_data->pattern_type == SDS_PAPPTERN_HYPERSLAB) {
        for (i = 0; i < ndim; i++) {
            global_start[i] = trace_data->h_pattern[0]->start[i];
            global_end[i]   = trace_data->h_pattern[nblock-1]->count[i];
            // count[] is actually the lower right corner
        }
        /*       // Print out global start end coordinate */
        /*       log_msg("[C] Global Start: "); */
        /*       for (i = 0; i < ndim; i++) */
        /*           log_msg("%d ",global_start[i]); */
        /*       log_msg("[C] Global End: "); */
        /*       for (i = 0; i < ndim; i++) */
        /*           log_msg("%d ",global_end[i]); */
    }



    int is_eligible           = 0;
    int is_full_overlap       = 0;
    int is_partial_overlap    = 0;
    int n_candidate           = 0; 
    int best_candidate        = -1;
    int candidate_index[MAX_NUM_CANDIDATE];

    ResponseTraceData * target_layout_metadata = NULL;

    // Lookup db to see if available layouts can satisfy current request
    // DB key is the file name, group, and dataset
    log_msg("Start to lookup database with KEY: [%s %s %s]", key.filename, key.group, key.dsetname);
    ret = read_metadata_record(g_sds_dbs.sds_metadata_dbp, &key, &value);
    if (ret == DB_NOTFOUND) { //find nothing
        log_msg("DB: No existing layout found for current request!");
        db_create_empty_record(g_sds_dbs.sds_metadata_dbp, &key, FILE_STATUS__IN_SERVICE);
        ret = read_metadata_record(g_sds_dbs.sds_metadata_dbp, &key, &value);
    }
    else if (ret == 0) {
        log_msg("DB returned n_layout_metadata [%d]", value.n_layout_metadata);

        // Step 1. fast candidate identification with start and end check
        for (i = 0; i < value.n_layout_metadata; i++) {
            // check if global start/end meets request (overlap or not)
            is_eligible     = 1;
            for (j = 0; j < ndim; j++) {
                if (global_start[j] != value.layout_metadata[i]->req_global_start[j])
                    is_eligible = 0;
                if (global_end[j] != value.layout_metadata[i]->req_global_end[j])
                    is_eligible = 0;
                if (is_eligible == 0)
                    break;
            }
            if (is_eligible == 1) 
                candidate_index[n_candidate++] = i;
        }
        log_msg("[%d] candidates eligible after global start/end check", n_candidate);

        // now candidate_index[] stores the index of all eligible layouts
        // Step 2. rank the candidates and return the best layout
        // TODO: currently only check full overlap using start[] and count[] of slab
        uint64_t tmp_request, tmp_layout;
        for (i = 0; i < n_candidate; i++) {
            is_full_overlap = 1;
            for (j = 0; j < nblock; j++) {
                if (is_full_overlap == 0) break;
                for (k = 0; k < ndim; k++) {
                    // start is full overlap?
                    tmp_request = trace_data->h_pattern[j]->start[k];
                    tmp_layout  = value.layout_metadata[candidate_index[i]]->opt_hyperslab[j]->start[k];
                    if (tmp_request != tmp_layout) { 
                        is_full_overlap = 0;
                        break;
                    }
                    // count is full overlap?
                    tmp_request = trace_data->h_pattern[j]->count[k];
                    tmp_layout  = value.layout_metadata[candidate_index[i]]->opt_hyperslab[j]->count[k];
                    if (tmp_request != tmp_layout) { 
                        is_full_overlap = 0;
                        break;
                    }
                }
            }

            if (is_full_overlap == 1) {
                // Current one (layout_metadata[i]) is full overlap to request
                best_candidate = i;
                break;
            }
        }


    }
    else {
        log_msg("Error happens when read metadata !");
        return NULL;
    }

    if (best_candidate < 0) {
        target_layout_metadata = NULL;
        log_msg("No existing layout is eligible for current client request");
    }
    else {
        log_msg("Client request has full overlap with layout [%d]!", best_candidate);
        print_layout_metadata(value.layout_metadata[best_candidate]);
        // Prepare data structure to send layout metadata back to client
        target_layout_metadata = convert_layout_metadata_to_responsetracedata(value.layout_metadata[best_candidate], nblock);
    }

    /* // Delete record for debug purposes */
    /* if (ret == 0 && value.n_layout_metadata > 0) { */
    /*     log_msg("Start to delete db record..."); */
    /*     ret = delete_metadata_record(g_sds_dbs.sds_metadata_dbp, &key); */
    /*     log_msg("Del ret=%d\n", ret); */
    /* } */

    // Merge patterns from different proc


    // Insert one layout for debug purposes
    /* insert_selection_as_layout_to_db(&key, &value, trace_data, nblock, ndim, global_start, global_end, sds_object); */


    free(global_start);
    free(global_end);

    return target_layout_metadata;
}

/*******************************************************************************
 * This file is part of Sedulus.
 * Copyright (c) 2022 Willem Elbers (whe@willemelbers.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

/* On-the-fly friends-of-friends halo finder */

#ifndef ANALYSIS_FOF_H
#define ANALYSIS_FOF_H

#include "particle.h"

struct fof_cell_list {
    long int offset;
    int cell;
};

struct fof_part_data {
    IntPosType x[3];
    long int root;
    long int global_offset;
    long int local_offset;
    long int global_root;
    long int halo_id;
};

struct halo_properties {
    long int global_id;
    double x_com[3];
    double v_com[3];
    double mass_fof;
    int npart;
};

int analysis_fof(struct particle *parts, double boxlen, long long int Ng,
                 long long int num_localpart, long long int max_partnum,
                 double linking_length, int halo_min_npart, int output_num,
                 double a_scale_factor);


static inline MPI_Datatype mpi_fof_data_type() {

    /* Construct an MPI data type from the constituent fields */
    MPI_Datatype particle_type;
    MPI_Datatype types[7] = {MPI_INTPOS_TYPE, MPI_LONG, MPI_LONG,
                             MPI_LONG, MPI_LONG, MPI_LONG};
    int lengths[7];
    MPI_Aint displacements[7];
    MPI_Aint base_address;
    struct fof_part_data temp;
    MPI_Get_address(&temp, &base_address);

    int counter = 0;

    /* Position */
    lengths[counter] = 3;
    MPI_Get_address(&temp.x[0], &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Root */
    lengths[counter] = 1;
    MPI_Get_address(&temp.root, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Global offset */
    lengths[counter] = 1;
    MPI_Get_address(&temp.global_offset, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Local offset */
    lengths[counter] = 1;
    MPI_Get_address(&temp.local_offset, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Global root */
    lengths[counter] = 1;
    MPI_Get_address(&temp.global_root, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Halo ID */
    lengths[counter] = 1;
    MPI_Get_address(&temp.halo_id, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Create the datatype */
    MPI_Type_create_struct(counter, lengths, displacements, types, &particle_type);
    MPI_Type_commit(&particle_type);

    return particle_type;
}

#endif
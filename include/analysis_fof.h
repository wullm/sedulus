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
#include "units.h"
#include "params.h"
#include "cosmology.h"

typedef int32_t CellIntType;
#define CELL_INT_BYTES 32

typedef uint32_t CellOffsetIntType;
#define MPI_OFFSET_TYPE MPI_UINT32_T
#define OFFSET_INT_BYTES 32

struct fof_cell_list {
    CellOffsetIntType offset;
    CellIntType cell;
};

static inline int row_major_cell(CellIntType j, CellIntType k, CellIntType N_cells) {
    return j * N_cells + k;
}

/* Determine the cell containing a given particle */
static inline CellIntType which_cell(IntPosType x[3], double int_to_cell_fac, CellIntType N_cells) {
    return row_major_cell((CellIntType) (int_to_cell_fac * x[1]),
                          (CellIntType) (int_to_cell_fac * x[2]), N_cells);
}

/* Order particles by their spatial cell index */
static inline int cellListSort(const void *a, const void *b) {
    struct fof_cell_list *ca = (struct fof_cell_list*) a;
    struct fof_cell_list *cb = (struct fof_cell_list*) b;

    return ca->cell >= cb->cell;
}

/* Compute the squared physical distance between two integer positions */
static inline double int_to_phys_dist2(const IntPosType ax[3],
                                       const IntPosType bx[3],
                                       double int_to_pos_fac) {

    /* Vector distance */
    const IntPosType dx = bx[0] - ax[0];
    const IntPosType dy = bx[1] - ax[1];
    const IntPosType dz = bx[2] - ax[2];

    /* Enforce boundary conditions */
    const IntPosType tx = (dx < -dx) ? dx : -dx;
    const IntPosType ty = (dy < -dy) ? dy : -dy;
    const IntPosType tz = (dz < -dz) ? dz : -dz;

    /* Convert to physical lengths */
    const double fx = tx * int_to_pos_fac;
    const double fy = ty * int_to_pos_fac;
    const double fz = tz * int_to_pos_fac;

    return fx * fx + fy * fy + fz * fz;
}

struct fof_part_data {
    /* The offset (local or global) of the root of the linked particle tree */
    long int root;
    /* The global offset of the corresponding particle in parts */
    long int global_offset;
};

/* If you add/change/remove fields to fof_part_first_exchange_data,
 * update the corresponding MPI data type below. */

struct fof_part_first_exchange_data {
    /* Integer positons of the particle */
    IntPosType x[3];
    /* The local offset */
    CellOffsetIntType local_offset;
};

/* If you add/change/remove fields to fof_part_second_exchange_data,
 * update the corresponding MPI data type below. */

struct fof_part_second_exchange_data {
    /* Integer positons of the particle */
    IntPosType x[3];
    /* The offset (local or global) of the root of the linked particle tree */
    long int root;
    /* The global offset of the corresponding particle in parts */
    long int global_offset;
};

/* If you add/change/remove fields to fof_halo,
 * update the corresponding MPI data type below. */

struct fof_halo {
    /* Global ID of the halo */
    long int global_id;
    /* Centre of mass of all FOF particles */
    double x_com[3];
    /* Total mass of all FOF particles */
    double mass_fof;
    /* Maximum distance between FOF particles and the CoM */
    double radius_fof;
    /* Shrinking sphere centre of mass */
    double x_com_inner[3];
    /* Number of linked FOF particles */
    int npart;
    /* Home rank of the halo */
    int rank;
};

int analysis_fof(struct particle *parts, double boxlen, long int N_cb,
                 long int N_nu, long long int Ng, long long int num_localpart,
                 long long int max_partnum, double linking_length,
                 int halo_min_npart, int output_num, double a_scale_factor,
                 const struct units *us, const struct physical_consts *pcs,
                 const struct cosmology *cosmo, struct params *pars,
                 const struct cosmology_tables *ctabs,
                 double dtau_kick, double dtau_drift);

/* The MPI data type of the FOF particle data (in the first exchange) */
static inline MPI_Datatype mpi_fof_part_first_type() {

    /* Construct an MPI data type from the constituent fields */
    MPI_Datatype particle_type;
    MPI_Datatype types[2] = {MPI_INTPOS_TYPE, MPI_OFFSET_TYPE};
    int lengths[2];
    MPI_Aint displacements[2];
    MPI_Aint base_address;
    struct fof_part_first_exchange_data temp;
    MPI_Get_address(&temp, &base_address);

    int counter = 0;

    /* Position */
    lengths[counter] = 3;
    MPI_Get_address(&temp.x[0], &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Local offset */
    lengths[counter] = 1;
    MPI_Get_address(&temp.local_offset, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Create the datatype */
    MPI_Type_create_struct(counter, lengths, displacements, types, &particle_type);
    MPI_Type_commit(&particle_type);

    return particle_type;
}

/* The MPI data type of the FOF particle data (in the second exchange) */
static inline MPI_Datatype mpi_fof_part_second_type() {

    /* Construct an MPI data type from the constituent fields */
    MPI_Datatype particle_type;
    MPI_Datatype types[3] = {MPI_INTPOS_TYPE, MPI_LONG, MPI_LONG};
    int lengths[3];
    MPI_Aint displacements[3];
    MPI_Aint base_address;
    struct fof_part_second_exchange_data temp;
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

    /* Create the datatype */
    MPI_Type_create_struct(counter, lengths, displacements, types, &particle_type);
    MPI_Type_commit(&particle_type);

    return particle_type;
}

/* The MPI data type of the FOF halo data */
static inline MPI_Datatype mpi_fof_halo_type() {

    /* Construct an MPI data type from the constituent fields */
    MPI_Datatype particle_type;
    MPI_Datatype types[7] = {MPI_LONG, MPI_DOUBLE, MPI_DOUBLE,
                             MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_INT};
    int lengths[7];
    MPI_Aint displacements[7];
    MPI_Aint base_address;
    struct fof_halo temp;
    MPI_Get_address(&temp, &base_address);

    int counter = 0;

    /* ID */
    lengths[counter] = 1;
    MPI_Get_address(&temp.global_id, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Position */
    lengths[counter] = 3;
    MPI_Get_address(&temp.x_com, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Mass */
    lengths[counter] = 1;
    MPI_Get_address(&temp.mass_fof, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Maximum distance to CoM */
    lengths[counter] = 1;
    MPI_Get_address(&temp.radius_fof, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Position (shrinking sphere) */
    lengths[counter] = 3;
    MPI_Get_address(&temp.x_com_inner, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Particle number */
    lengths[counter] = 1;
    MPI_Get_address(&temp.npart, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* MPI Home Rank */
    lengths[counter] = 1;
    MPI_Get_address(&temp.rank, &displacements[counter]);
    displacements[counter] = MPI_Aint_diff(displacements[counter], base_address);
    counter++;

    /* Create the datatype */
    MPI_Type_create_struct(counter, lengths, displacements, types, &particle_type);
    MPI_Type_commit(&particle_type);

    return particle_type;
}

#endif

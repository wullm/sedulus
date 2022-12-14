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

/* On-the-fly power spectra */

#ifndef ANALYSIS_POWSPEC_H
#define ANALYSIS_POWSPEC_H

#include "particle.h"
#include "units.h"
#include "params.h"
#include "cosmology.h"
#include "distributed_grid.h"
#include "mass_deposit.h"

double calc_shot_noise(double boxlen, long int N_cb, long int N_nu,
                       double nu_factor, enum grid_type gtype);
int analysis_powspec(struct distributed_grid *dgrid, int output_num,
                     double a_scale_factor, FourierPlanType r2c,
                     const struct units *us, const struct physical_consts *pcs,
                     const struct cosmology *cosmo, struct params *pars,
                     enum grid_type gtype, double shot_noise);

#endif

/* ======================================================================
   MDCORE - Interatomic potential library
   https://github.com/pastewka/mdcore
   Lars Pastewka, lars.pastewka@iwm.fraunhofer.de, and others
   See the AUTHORS file in the top-level MDCORE directory.

   Copyright (2005-2013) Fraunhofer IWM
   This software is distributed under the GNU General Public License.
   See the LICENSE file in the top-level MDCORE directory.
   ====================================================================== */

#ifndef __LOGGING_H
#define __LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

void prscrlog(char *msg, ...);
void prlog(char *msg, ...);

void c_prscrlog(char *msg);
void c_prlog(char *msg);

#ifdef __cplusplus
}
#endif

#endif

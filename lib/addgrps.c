// SPDX-FileCopyrightText: 1989-1994, Julianne Frances Haugh
// SPDX-FileCopyrightText: 1996-1998, Marek Michałkiewicz
// SPDX-FileCopyrightText: 2001-2006, Tomasz Kłoczko
// SPDX-FileCopyrightText: 2007-2009, Nicolas François
// SPDX-FileCopyrightText: 2024, Alejandro Colomar <alx@kernel.org>
// SPDX-License-Identifier: BSD-3-Clause


#include <config.h>

#if defined (HAVE_SETGROUPS) && ! defined (USE_PAM)

#include "prototypes.h"
#include "defines.h"

#include <errno.h>
#include <grp.h>
#include <stdio.h>
#include <string.h>

#include "alloc/malloc.h"
#include "alloc/reallocf.h"
#include "search/l/lsearch.h"
#include "shadowlog.h"

#ident "$Id$"


/*
 * Add groups with names from LIST (separated by commas or colons)
 * to the supplementary group set.  Silently ignore groups which are
 * already there.
 */
int
add_groups(const char *list)
{
	GETGROUPS_T *grouplist;
	char *g, *p;
	char buf[1024];
	FILE *shadow_logfd = log_get_logfd();
	size_t  ngroups;

	if (strlen (list) >= sizeof (buf)) {
		errno = EINVAL;
		return -1;
	}
	strcpy (buf, list);

	ngroups = getgroups(0, NULL);
	if (ngroups == -1)
		return -1;

	grouplist = MALLOC(ngroups, GETGROUPS_T);
	if (grouplist == NULL)
		return -1;

	ngroups = getgroups(ngroups, grouplist);
	if (ngroups == -1)
		goto free_gids;

	p = buf;
	while (NULL != (g = strsep(&p, ",:"))) {
		struct group  *grp;

		grp = getgrnam(g); /* local, no need for xgetgrnam */
		if (NULL == grp) {
			fprintf(shadow_logfd, _("Warning: unknown group %s\n"), g);
			continue;
		}

		grouplist = REALLOCF(grouplist, ngroups + 1, GETGROUPS_T);
		if (grouplist == NULL)
			return -1;

		LSEARCH(&grp->gr_gid, grouplist, &ngroups);
	}

	if (setgroups(ngroups, grouplist) == -1) {
		fprintf(shadow_logfd, "setgroups: %s\n", strerror(errno));
		goto free_gids;
	}

	free (grouplist);
	return 0;

free_gids:
	free(grouplist);
	return -1;
}
#else				/* HAVE_SETGROUPS && !USE_PAM */
extern int ISO_C_forbids_an_empty_translation_unit;
#endif				/* HAVE_SETGROUPS && !USE_PAM */


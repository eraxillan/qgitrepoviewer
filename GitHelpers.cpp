/**
 * @file
 * @brief Implementation of git helpers classes
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 */

#include "GitHelpers.h"

#include <QDebug>
#include <QCoreApplication>
#include <QFile>

#include <git2.h>

using namespace QGitRepoViewer;

/// Custom user-defined git error code (all standart errors have negative codes)
#define GIT_USER_ERROR 1

// CGitReference implementation /////////////////////////////////////////////////////////////////////

// CGitRepository implementation ////////////////////////////////////////////////////////////////////

CGitRepository::CGitRepository (): m_repo (NULL)
{}

CGitRepository::CGitRepository (const QString& _path): m_repo (NULL)
{
	open (_path, true);
}

CGitRepository::~CGitRepository ()
{
	close ();
}

bool
CGitRepository::open (const QString& _path, bool _discover)
{
	bool result = false;

	close ();

	//
	// Search for git repository (".git" directory) in _path directory
	// and recursively in its parent ones
	//
	int error_code;
	if (_discover)
	{
		error_code = git_repository_open_ext (&m_repo, QFile::encodeName (_path),
											  GIT_REPOSITORY_OPEN_CROSS_FS, NULL);
		if (error_code == GIT_OK)
			result = true;
		else
			setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "searching for repository through file system"));
	}
	else
	{
		error_code = git_repository_open (&m_repo, QFile::encodeName (_path));
		if (error_code == GIT_OK)
			result = true;
		else
			setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "opening repository"));
	}

	return result;
}

bool
CGitRepository::isOpened () const
{
	return (m_repo != NULL);
}

bool
CGitRepository::init (const QString& _path, bool _bare)
{
	bool result = false;

	close ();

	int error_code = git_repository_init (&m_repo, QFile::encodeName (_path), _bare);
	if (error_code == GIT_OK)
		result = true;
	else
		setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "creating repository"));

	return result;
}

void
CGitRepository::close ()
{
	if (m_repo)
	{
		git_repository_free (m_repo);
		m_repo = NULL;
	}
}

QString
CGitRepository::lastError () const
{
	return m_last_error;
}

void
CGitRepository::setLastError (int _code, const QString& _action)
{
	m_last_error.clear ();

	if (_code != GIT_OK)
	{
		const git_error* error = giterr_last ();
		m_last_error = QCoreApplication::translate (TR_CONTEXT, "Error with code %1 during %2:\n %3")
					   .arg (_code)
					   .arg (_action)
					   .arg ((error && error->message) ? QString::fromUtf8 (error->message)
													   : QCoreApplication::translate (TR_CONTEXT, "<Unknown>"));
	}
	else if (_code == GIT_USER_ERROR)
	{
		const git_error* error = giterr_last ();
		m_last_error = QCoreApplication::translate (TR_CONTEXT, "Error during %1:\n %3")
					   .arg (_action)
					   .arg ((error && error->message) ? QString::fromUtf8 (error->message)
													   : QCoreApplication::translate (TR_CONTEXT, "<Unknown>"));
	}
}

QString
CGitRepository::path () const
{
	QString path;
	if (m_repo)
		path = QString::fromUtf8 (git_repository_path (m_repo));

	return path;
}

CGitRepository::State
CGitRepository::currState () const
{
	State st = STATE_NONE;

	if (m_repo)
	{
		switch (git_repository_state (m_repo))
		{
			case GIT_REPOSITORY_STATE_MERGE:					st = STATE_MERGE;					break;
			case GIT_REPOSITORY_STATE_REVERT:					st = STATE_REVERT;					break;
			case GIT_REPOSITORY_STATE_CHERRY_PICK:				st = STATE_CHERRY_PICK;				break;
			case GIT_REPOSITORY_STATE_BISECT:					st = STATE_BISECT;					break;
			case GIT_REPOSITORY_STATE_REBASE:					st = STATE_REBASE;					break;
			case GIT_REPOSITORY_STATE_REBASE_INTERACTIVE:		st = STATE_REBASE_INTERACTIVE;		break;
			case GIT_REPOSITORY_STATE_REBASE_MERGE:				st = STATE_REBASE_MERGE;			break;
			case GIT_REPOSITORY_STATE_APPLY_MAILBOX:			st = STATE_APPLY_MAILBOX;			break;
			case GIT_REPOSITORY_STATE_APPLY_MAILBOX_OR_REBASE:	st = STATE_APPLY_MAILBOX_OR_REBASE; break;
			default: break;
		}
	}

	return st;
}

bool
CGitRepository::isBare () const
{
	bool is_bare = false;

	if (m_repo)
		is_bare = (git_repository_is_bare (m_repo) != 0);

	return is_bare;
}

bool
CGitRepository::isEmpty () const
{
	bool is_empty = false;

	if (m_repo)
		is_empty = (git_repository_is_empty (m_repo) != 0);

	return is_empty;
}

bool
CGitRepository::isShallow () const
{
	bool is_shallow = false;

	if (m_repo)
		is_shallow = (git_repository_is_shallow (m_repo) != 0);

	return is_shallow;
}

QString
CGitRepository::workDir () const
{
	QString work_dir;

	if (m_repo)
		work_dir = QString::fromUtf8 (git_repository_workdir (m_repo));

	return work_dir;
}

bool
CGitRepository::setWorkDir (const QString& _path)
{
	bool result = false;

	if (m_repo)
	{
		int error_code = git_repository_set_workdir (m_repo, QFile::encodeName (_path), TRUE);
		if (error_code == GIT_OK)
			result = true;
		else
			setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "setting new working directory %1 for repository")
						  .arg (_path));
	}

	return result;
}

QString
CGitRepository::currNamespace () const
{
	QString curr_namespace;

	if (m_repo)
		curr_namespace = QString::fromUtf8 (git_repository_get_namespace (m_repo));

	return curr_namespace;
}

bool
CGitRepository::setCurrNamespace (const QString& _namespace)
{
	bool result = false;

	if (m_repo)
	{
		int error_code = git_repository_set_namespace (m_repo, QFile::encodeName (_namespace));
		if (error_code == GIT_OK)
			result = true;
		else
			setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "setting new namespace %1 for repository")
						  .arg (_namespace));
	}

	return result;
}

//
//
//

/**
 * @brief Callback function for walking through all GIT repository local branches
 * @param _branch_name Human-readable name of the branch
 * @param _branch_type Type of branch (local or remote)
 * @param _payload Pointer to some user specified data
 * @return GIT_OK if there is no error, and error code if one occurred
 */
static int gitLocalBranchCb (const char* _branch_name, git_branch_t _branch_type, void* _payload)
{
	QList<CGitBranch>* branches = static_cast <QList<CGitBranch>*> (_payload);
	Q_ASSERT (branches);
	branches->append (CGitBranch (_branch_name, (_branch_type == GIT_BRANCH_REMOTE)));

	return GIT_OK;
}

QString gitObjectIdAsString (const git_oid* _oid)
{
	char commit_id [41] = {0};
	git_oid_fmt (commit_id, _oid);
	return commit_id;
}

QList<CGitBranch>
CGitRepository::enumBranches (bool _local_only)
{
	QList<CGitBranch> branches;

	if (!m_repo)
		return branches;

	//
	// Determine that branches user wants: only local or locan and remote
	//
	unsigned int list_flags = (_local_only ? GIT_BRANCH_LOCAL : (GIT_BRANCH_LOCAL | GIT_BRANCH_REMOTE));

	//
	// Obtain the list of all specified repository branches
	//
	int error_code = git_branch_foreach (m_repo, list_flags, &gitLocalBranchCb, &branches);
	if (error_code == GIT_OK)
	{
		typedef QList <QList<CGitBranch>::iterator> BranchIteratorList;
		BranchIteratorList remove_candidates;
		//
		// Check each branch for correctness and whether it is the current one in repository
		//
		QList<CGitBranch>::iterator iBranch = branches.begin ();
		for ( ; iBranch != branches.end (); ++iBranch)
		{
			git_reference* git_branch = NULL;
			error_code = git_branch_lookup (&git_branch, m_repo, QFile::encodeName (iBranch->m_name),
											(iBranch->m_is_remote ? GIT_BRANCH_REMOTE : GIT_BRANCH_LOCAL));
			if (error_code == GIT_OK)
			{
				//
				// Obtain branch SHA-1 id
				//
				git_oid oid;
				git_reference* resolved = NULL;
				error_code = git_reference_resolve (&resolved, git_branch);
				if (error_code == GIT_OK)
				{
					error_code = git_reference_name_to_id (&oid, m_repo, git_reference_name (resolved));
					if (error_code == GIT_OK)
						iBranch->m_id = gitObjectIdAsString (&oid);
					else
					{
						setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "obtaining branch SHA-1 id"));
						break;
					}
				}
				else
				{
					setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "resolving branch target"));
					break;
				}

				//
				// Obtain the branch shorthand name for showing to user in GUI
				//
				iBranch->m_shorthand_name = git_reference_shorthand (resolved);

				//
				// Check whether branch shorthand name is unique
				//
				bool unique_branch = true;
				QList<CGitBranch>::iterator iTempBranch = branches.begin ();
				for ( ; iTempBranch != branches.end (); ++iTempBranch)
				{
					if (iTempBranch == iBranch)
						continue;

					if (iTempBranch->m_shorthand_name == iBranch->m_shorthand_name)
					{
						unique_branch = false;
						break;
					}
				}

				if (! unique_branch)
					remove_candidates.append (iBranch);

				//
				// Check whether the current branch is the HEAD of repository
				//
				iBranch->m_is_head = (git_branch_is_head (git_branch) == TRUE);

				//
				// For local branches try to determine upstream ones (if they are set)
				//
				if (!iBranch->m_is_remote)
				{
					git_reference* upstream_branch = NULL;
					error_code = git_branch_upstream (&upstream_branch, resolved);
					if (error_code == GIT_OK)
						iBranch->m_upstream_branch = git_reference_shorthand (upstream_branch);
					else if (error_code != GIT_ENOTFOUND)
					{
						setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "determing local branch upstream one"));
						break;
					}
					else
						error_code = GIT_OK;
				}

				//
				// Obtaining remote repository name
				//
				/*if (iBranch->m_remote)
				{
					int len = git_branch_remote_name (NULL, 0, m_repo, git_reference_name (resolved));
					if (len > 0)
					{
						char* buf = new char [len];
						int ec = git_branch_remote_name (buf, len, m_repo, git_reference_name (resolved));
						qDebug () << buf;
						qDebug () << ec;
						delete [] buf;
					}
				}*/

				//
				// TODO: obtain upstream branch
				//

				git_reference_free (git_branch);
			}
			else
			{
				setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "searching for the specified branch"));
				break;
			}
		}

		//
		// Remove all duplicated branches
		//
		BranchIteratorList::iterator iRemovingBranch = remove_candidates.begin ();
		for ( ; iRemovingBranch != remove_candidates.end (); ++iRemovingBranch)
			branches.erase ((*iRemovingBranch));
	}
	else
		setLastError (error_code, QCoreApplication::translate (TR_CONTEXT, "iterating over the git branches"));

	return branches;
}

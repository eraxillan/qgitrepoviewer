/**
 * @file
 * @brief Table data model for representing commits of git repository implementation
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (С) SpectrumSoft, 2010-2013
 */
#include "CCommitModel.h"

#include <QFile>
#include <QTranslator>
#include <QDateTime>
#include <QMessageBox>

#include <git2.h>

#define GIT_USER_ERROR 1

static QString getGitError (int _error_code, const QString& _action)
{
	QString git_error_str;
	if (_error_code != GIT_OK)
	{
		const git_error* error = giterr_last ();
		git_error_str = QString ("Error with code %1 during %2:\n %3")
						.arg (_error_code)
						.arg (_action)
						.arg ((error && error->message) ? QString::fromUtf8 (error->message) : "<Unknown>");
	}
	else if (_error_code == GIT_USER_ERROR)
	{
		const git_error* error = giterr_last ();
		git_error_str = QString ("Error during %1:\n %3")
						.arg (_action)
						.arg ((error && error->message) ? QString::fromUtf8 (error->message) : "<Unknown>");
	}

	return git_error_str;
}

static void showGitError (QWidget* _parent, int _error_code, const QString& _action)
{
	QMessageBox::critical (_parent, QTranslator::tr ("VCS error"), getGitError (_error_code, _action));
}

static git_commit* resolveCommit (const QString& _commit_id, git_repository* _repo)
{
	git_commit* commit = NULL;
	git_oid oid;
	git_oid_fromstr (& oid, QFile::encodeName (_commit_id));
	int error_code = git_commit_lookup (& commit, _repo, &oid);
	if (error_code != GIT_OK)
	{
		commit = NULL;
		showGitError (NULL, error_code, QTranslator::tr ("looking up commit"));
	}

	return commit;
}

/// Returns the short of full changelog of specified commit
static QString commitLog (const QString& _commit_id, git_repository* _repo, bool _short = false)
{
	QString commit_log;

	git_commit* commit = resolveCommit (_commit_id, _repo);
	if (commit)
	{
		commit_log = QString::fromUtf8 (git_commit_message (commit));
		commit_log.replace ("\n", "<br>");
		if (commit_log.right (4) == "<br>")
			commit_log.remove (commit_log.length() - 4, 4);

		if (_short)
		{
			int line_end = commit_log.indexOf ("<br>");
			if (line_end != -1)
				commit_log = commit_log.left (line_end);
		}

		git_commit_free (commit);
	}

	return commit_log;
}

/// Returns the author (name + email) of specified commit
static QString commitAuthor (const QString& _commit_id, git_repository* _repo)
{
	QString commit_author;
	git_commit* commit = resolveCommit (_commit_id, _repo);
	if (commit)
	{
		const git_signature* author = git_commit_author (commit);
		if (author)
			commit_author = QString::fromUtf8 (author->name)
							+ QString (" <") + QString::fromUtf8 (author->email) + ">";
	}

	return commit_author;
}

/// Returns the date of specified commit
static QString commitDate (const QString& _commit_id, git_repository* _repo)
{
	QString commit_time;

	git_commit* commit = resolveCommit (_commit_id, _repo);
	if (commit)
	{
		time_t time = git_commit_time (commit);
		QDateTime dateTime;
		dateTime.setTime_t (time);
		commit_time = dateTime.toString ();
	}

	return commit_time;
}

namespace
{
	/// gitTagCb parameters structure
	struct CTagCbContext
	{
		git_repository* m_repo;
		const git_oid* m_commit_oid;
		QStringList m_commit_tags;
	};
}

/// Callback for git_tag_foreach function for git commit tags walking
static int gitTagCb (const char* _name, git_oid* _oid, void* _payload)
{
	CTagCbContext* context = (CTagCbContext*) _payload;
	Q_ASSERT (context);

	//
	// Если идентификатор коммита равен указанному
	//
	if (git_oid_cmp (context->m_commit_oid, _oid) == 0)
	{
		//
		// Если удалось найти ссылку (а тег это тоже ссылка) с указанным именем
		//
		git_reference* ref = NULL;
		if (git_reference_lookup (& ref, context->m_repo, _name) == GIT_OK)
		{
			//
			// Запоминаем тег в переданной структуре
			//
			context->m_commit_tags.append (git_reference_shorthand (ref));

			git_reference_free (ref);
		}
	}

	return GIT_OK;
}

/**
 * @brief Return the list of tags of specified commit
 */
static QStringList commitTags (const QString& _commit_id, git_repository* _repo)
{
	CTagCbContext context;
	git_commit* commit = resolveCommit (_commit_id, _repo);
	if (commit)
	{
		context.m_repo = _repo;
		context.m_commit_oid = git_commit_id (commit);
		if (git_tag_foreach (_repo, & gitTagCb, (void*)&context) != GIT_OK)
			Q_ASSERT (0);
	}

	return context.m_commit_tags;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace QGitRepoViewer;

CCommitTableModel::CCommitTableModel (QObject* _parent):
	QAbstractTableModel (_parent),
	m_repo (NULL)
{}

CCommitTableModel::~CCommitTableModel ()
{
	if (m_repo)
		git_repository_free (m_repo);
}

void CCommitTableModel::setGitRepo (const QString& _repo_path)
{
	int error_code = git_repository_open_ext (& m_repo, QFile::encodeName (_repo_path),
											  GIT_REPOSITORY_OPEN_CROSS_FS, NULL);
	if (error_code != GIT_OK)
		showGitError (NULL, error_code, tr ("opening repository"));
}

int CCommitTableModel::commitIndex (const QString _commit_id) const
{
	return m_commits.indexOf (_commit_id);
}

void CCommitTableModel::setCommitList (const QString& _branch_name)
{
	// Clear current commit ids list
	m_commits.clear ();

	// Search for brach with specified name in git repository
	git_reference* git_branch = NULL;
	int error_code = git_branch_lookup (& git_branch, m_repo,
										QFile::encodeName (_branch_name), GIT_BRANCH_LOCAL);
	if (error_code == GIT_OK)
	{
		Q_ASSERT (git_reference_type (git_branch) == GIT_REF_OID);

		// Obtain the branch id
		const git_oid* branch_oid = git_reference_target (git_branch);
		if (branch_oid)
		{
			// Obtain the HEAD commit object
			git_object* branch_head = NULL;
			error_code = git_reference_peel (& branch_head, git_branch, GIT_OBJ_COMMIT);
			if (error_code == GIT_OK)
			{
				Q_ASSERT (git_object_type (branch_head) == GIT_OBJ_COMMIT);

				// Obtain the HEAD commit id
				const git_oid* branch_head_oid = git_object_id (branch_head);
				Q_ASSERT (branch_head_oid != NULL);

				// Create the commit iterator object
				git_revwalk* rev_walk = NULL;
				error_code = git_revwalk_new (& rev_walk, m_repo);
				if (error_code == GIT_OK)
				{
					// Setup the commit iterator
					git_revwalk_sorting (rev_walk, GIT_SORT_TOPOLOGICAL | GIT_SORT_TIME);
					error_code = git_revwalk_push (rev_walk, branch_head_oid);
					if (error_code == GIT_OK)
					{
						// Walk through all branch commits
						git_oid oid;
						git_commit* wcommit = NULL;
						while ((git_revwalk_next (&oid, rev_walk)) == GIT_OK)
						{
							error_code = git_commit_lookup (& wcommit, m_repo, &oid);
							if (error_code == GIT_OK)
							{
								// Add current commit id to the list
								char commit_id [41] = {0};
								git_oid_fmt (commit_id, & oid);
								m_commits.append (commit_id);

								git_commit_free (wcommit);
							}
							else
								showGitError (NULL, error_code, tr ("looking up commit during revwalk"));
						}
					}
					else
						showGitError (NULL, error_code, tr ("setting up start commit for revision walking"));

					git_revwalk_free (rev_walk);
				}
				else
					showGitError (NULL, error_code, tr ("allocating libgit2 revision walking object"));

				git_object_free (branch_head);
			}
			else
				showGitError (NULL, error_code, tr ("obtaining HEAD commit of branch"));
		}
		else
			showGitError (NULL, GIT_USER_ERROR, tr ("resolving branch as reference"));

		git_reference_free (git_branch);
	}
	else
		showGitError (NULL, error_code, tr ("looking up local branch"));

	// Reset model data in all attached views
	reset ();
}

bool CCommitTableModel::empty () const
{
	return m_commits.isEmpty ();
}

int CCommitTableModel::rowCount (const QModelIndex& _parent) const
{
	Q_UNUSED (_parent);
	return m_commits.count ();
}

int CCommitTableModel::columnCount (const QModelIndex& _parent) const
{
	Q_UNUSED (_parent);
	return _ColumntCount;
}

QVariant CCommitTableModel::data (const QModelIndex& _index, int _role) const
{
	if (_index.isValid ())
	{
		// Obtain the commit id (SHA-1 hash)
		const QString& commit_id = m_commits [_index.row ()];

		// Return commit id for the custom role
		// TODO: unusable?
		if (_role == Qt::UserRole)
			return commit_id;

		switch (_role)
		{
			case Qt::DisplayRole:
				switch (_index.column ())
				{
					case _ShortLogColumn:
					{
						// Show commit short log and tags
						QStringList tags = commitTags (commit_id, m_repo);
						QString tag_string;
						foreach (const QString& tag, tags)
							tag_string += "[" + tag + "] ";
						return (tag_string + commitLog (commit_id, m_repo, true));
					}

					case _AuthorColumn:
						return commitAuthor (commit_id, m_repo);

					case _DateColumn:
						return commitDate (commit_id, m_repo);
				}

			case Qt::ToolTipRole:
				return commitLog (commit_id, m_repo);

			case Qt::BackgroundRole:
			{
				// Change background color to yellow for the tagged commits
				if ((_index.column () == _ShortLogColumn) && !commitTags (commit_id, m_repo).isEmpty ())
					return QBrush (Qt::yellow);

				break;
			}

			default: break;
		}
	}

	return QVariant ();
}

QVariant CCommitTableModel::headerData (int _section, Qt::Orientation _orientation, int _role) const
{
	Q_UNUSED (_orientation);

	if (_role == Qt::DisplayRole)
	{
		switch (_section)
		{
			case _ShortLogColumn:
				return tr ("Short Log");

			case _AuthorColumn:
				return tr ("Author");

			case _DateColumn:
				return tr ("Date");
		}
	}

	return QVariant ();
}

QModelIndexList CCommitTableModel::match (const QModelIndex& _start, int _role, const QVariant& _value,
						  int _hits, Qt::MatchFlags _flags) const
{
	// NOTE: match() from base class due to recursive flag cause stack overflow!
	// So we forced to remove this flag manually, because table have flat structure and
	// doesn't need recursion using at all
	return QAbstractTableModel::match (_start, _role, _value, _hits, (_flags & ~Qt::MatchRecursive));
}

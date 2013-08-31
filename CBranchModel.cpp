/**
 * @file
 * @brief Implemetation of custom item data model for branches of git SCM repository
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (С) SpectrumSoft, 2010-2013
 */
#include "CBranchModel.h"

#include <QFile>
#include <QTranslator>
#include <QDateTime>
#include <QMessageBox>

#include <git2.h>

/// Custom git error code
#define GIT_USER_ERROR 1

/**
 * @brief Возвращает строковое представление для указанного кода ошибки git
 * @param _error_code Код ошибки git
 * @param _action Действие, при котором произошла ошибка
 */
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

/**
 * @brief Сообщает об указанной ошибке git при указанном действии в модальном диалоге
 */
static void showGitError (QWidget* _parent, int _error_code, const QString& _action)
{
	QMessageBox::critical (_parent, QTranslator::tr ("VCS error"), getGitError (_error_code, _action));
}

/**
 * @brief Callback function for walking through all Git repository local branches
 * @param _branch_name Human-readable name of the branch
 * @param _branch_type Type of branch (must be local always)
 * @param _payload Pointer to some user specified data
 * @return GIT_OK if there is no error, and error code if one occurred
 */
static int gitLocalBranchCb (const char* _branch_name, git_branch_t _branch_type, void* _payload)
{
	Q_UNUSED (_branch_type);

	QStringList* local_branches = ((QStringList*) _payload);
	Q_ASSERT (local_branches);
	local_branches->append (_branch_name);

	return GIT_OK;
}

/// Obtain the list of all specified repository local branches
static QStringList gitAllBranches (git_repository* _repo)
{
	QStringList local_branches;

	// Obtain the list of all specified repository local branches
	int error_code = git_branch_foreach (_repo, GIT_BRANCH_LOCAL, & gitLocalBranchCb, & local_branches);
	if (error_code == GIT_OK)
	{
		// Check each branch for correctness and whether it is current
		QStringList::iterator iBranch = local_branches.begin ();
		for ( ; iBranch != local_branches.end (); ++iBranch)
		{
			git_reference* git_branch = NULL;
			if (git_branch_lookup (& git_branch, _repo, QFile::encodeName (*iBranch), GIT_BRANCH_LOCAL) == GIT_OK)
			{
				*iBranch = (git_branch_is_head (git_branch) == 1) ? ("* " + *iBranch) : ("   " + *iBranch);

				Q_ASSERT (git_reference_type (git_branch) == GIT_REF_OID);
				git_reference_free (git_branch);
			}
			else
				showGitError (NULL, error_code, QTranslator::tr ("searching for the specified branch"));
		}
	}
	else
		showGitError (NULL, error_code, QTranslator::tr ("iterating over the git branches"));

	return local_branches;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace QGitRepoViewer;

CBranchListModel::CBranchListModel (QObject* _parent):
	QAbstractItemModel (_parent)
{}

CBranchListModel::~CBranchListModel ()
{}

void CBranchListModel::loadFromGit (const QString& _repo_path)
{
	// Search git repository (".git" directory) in _repo_path and recursively in its parent directories
	git_repository* repo = NULL;
	int error_code = git_repository_open_ext (& repo, QFile::encodeName (_repo_path),
											  GIT_REPOSITORY_OPEN_CROSS_FS, NULL);
	if (error_code == GIT_OK)
	{
		// Obtain the list of local git repository branches
		m_branches.clear ();
		m_branches = gitAllBranches (repo);

		// Free the memory captured by repository object
		git_repository_free (repo);
	}
	else
		showGitError (NULL, error_code, QTranslator::tr ("opening repository"));

	// Reset model in all attached views
	reset ();
}

bool CBranchListModel::empty () const
{
	return m_branches.empty ();
}

int CBranchListModel::rowCount (const QModelIndex& _parent) const
{
	Q_UNUSED (_parent);
	return m_branches.count ();
}

int CBranchListModel::columnCount (const QModelIndex& _parent) const
{
	Q_UNUSED (_parent);
	return 1;
}

QVariant CBranchListModel::data (const QModelIndex& _index, int _role) const
{
	Q_UNUSED (_role);

	if (_index.isValid ())
	{
		Q_ASSERT ((_index.row () >= 0) && (_index.row () < m_branches.size ()));

		// Return previosly found branch
		return m_branches [_index.row ()];
	}

	return QVariant ();
}

QModelIndex CBranchListModel::index (int _row, int _column, const QModelIndex& _parent) const
{
	Q_UNUSED (_column);
	Q_UNUSED (_parent);

	// List item have only one index - row one
	return createIndex (_row, 0);
}

QModelIndex CBranchListModel::parent (const QModelIndex& _child) const
{
	Q_UNUSED (_child);

	// List has flat structure and haven't child-parent relations at all
	return QModelIndex ();
}

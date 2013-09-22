/**
 * @file
 * @brief Implemetation of custom item data model for branches of GIT SCM repository
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (С) SpectrumSoft, 2013
 */
#include "CBranchModel.h"

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QTranslator>
#include <QDateTime>
#include <QMessageBox>
#include <QIcon>

#include "GitHelpers.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace QGitRepoViewer;

CBranchListModel::CBranchListModel (QObject* _parent):
	QAbstractListModel (_parent)
{}

CBranchListModel::~CBranchListModel ()
{}

void
CBranchListModel::showLastGitError ()
{
	if (m_repo.isOpened ())
	{
		QMessageBox::critical (NULL, QCoreApplication::translate (TR_CONTEXT, "GIT error"),
							   m_repo.lastError ());
	}
}

void
CBranchListModel::loadFromGit (const QString& _repo_path)
{
	beginResetModel ();

	//
	// Search git repository (".git" directory) in _repo_path and recursively in its parent directories
	//
	if (m_repo.open (_repo_path, true))
	{
		//
		// Obtain the list of local git repository branches
		//
		m_branches.clear ();
		// FIXME: allow remote branches to working correctly too
		m_branches = m_repo.enumBranches (/*false*/);
		if (! m_repo.lastError ().isEmpty ())
			qWarning () << m_repo.lastError ();

		//
		// Free the repository object
		//
		m_repo.close ();
	}
	else
		qWarning () << m_repo.lastError ();

	//
	// Reset model in all attached views
	//
	endResetModel ();
}

bool
CBranchListModel::empty () const
{
	return m_branches.empty ();
}

//bool
//CBranchListModel::createBranch (const QString& _name, const QString& _commit_id)
//{
//	git_reference* new_branch = NULL;
//	git_branch_create (& new_branch, m_repo, _name.toUtf8 ().data (), target_commit, FALSE);
//}

/////////////////////////////////////////////////////////////////////////////////////////////////////

int
CBranchListModel::rowCount (const QModelIndex& _parent) const
{
	Q_UNUSED (_parent);

	return m_branches.count ();
}

QVariant
CBranchListModel::data (const QModelIndex& _index, int _role) const
{
	Q_UNUSED (_role);

	if (_index.isValid ())
	{
		Q_ASSERT ((_index.row () >= 0) && (_index.row () < m_branches.size ()));

		switch (_index.column ())
		{
			case 0:
			{
				switch (_role)
				{
					case Qt::DisplayRole:
						return m_branches [_index.row ()].m_shorthand_name;

					case Qt::ToolTipRole:
					{
						QString tooltip;
						if (m_branches [_index.row ()].m_is_head)
						{
							tooltip += QCoreApplication::translate (TR_CONTEXT, "Repository HEAD branch");
							tooltip += LINEBREAK;
						}
						if (m_branches [_index.row ()].m_is_remote)
						{
							tooltip += QCoreApplication::translate (TR_CONTEXT, "Remote branch");
							tooltip += LINEBREAK;
						}
						else
						{
							tooltip += QCoreApplication::translate (TR_CONTEXT, "Local branch");
							tooltip += LINEBREAK;

							if (! m_branches [_index.row ()].m_upstream_branch.isEmpty ())
							{
								tooltip += QCoreApplication::translate (TR_CONTEXT, "Upstream branch name: %1")
										   .arg (m_branches [_index.row ()].m_upstream_branch);
								tooltip += LINEBREAK;
							}
						}

						tooltip += QString ("Branch id is %1").arg (m_branches [_index.row ()].m_id);

						return tooltip;
					}

					case Qt::DecorationRole:
					{
						if (m_branches [_index.row ()].m_is_head)
							return QIcon (":/qgitrepoviewer/icons/asterisk_orange.png");

						if (m_branches [_index.row ()].m_is_remote)
							return QIcon (":/qgitrepoviewer/icons/remote.png");
						else
							return QIcon (":/qgitrepoviewer/icons/computer.png");
					}

					default: break;
				}
			}

			default: break;
		}
	}

	return QVariant ();
}


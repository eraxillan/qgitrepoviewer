/**
 * @file
 * @brief Interface of custom item data model for branches of git SCM repository
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (C) SpectrumSoft, 2010-2013
 */
#ifndef __QGITREPOVIEWER_CBRANCHMODEL_H
#define __QGITREPOVIEWER_CBRANCHMODEL_H

#include <QStringList>
#include <QAbstractTableModel>

#include "GitHelpers.h"

namespace QGitRepoViewer
{
	/// Simple read-only list data model for viewing local branches of git SCM repository
	class CBranchListModel : public QAbstractListModel
	{
		Q_OBJECT

		/// The list of local branch names of git repository
		QList<CGitBranch> m_branches;

		CGitRepository m_repo;

		void showLastGitError ();

	public:
		CBranchListModel (QObject* _parent = 0);
		~CBranchListModel ();

		/// Load the list of local branches of specified git repository
		void loadFromGit (const QString& _repo_path);

		/// Check for existance of at least one branch in git repository
		bool empty () const;

		/// Creates the new branch pointing to specified commit (to HEAD by default)
		//bool createBranch (const QString& _name, const QString& _commit_id = QString ());

	public:
		/// @name Implementation of QAbstractItemModel interface
		/** @{*/
		int rowCount (const QModelIndex& _parent = QModelIndex ()) const;
		QVariant data (const QModelIndex& _index, int _role = Qt::DisplayRole) const;
		/** @}*/
	};
}

#endif // __QGITREPOVIEWER_CBRANCHMODEL_H

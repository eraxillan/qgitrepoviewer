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

namespace QGitRepoViewer
{
	/// Custom item data model for local branches of git SCM repository
	class CBranchListModel : public QAbstractItemModel
	{
		Q_OBJECT

		/// The list of local branch names of git repository
		QStringList m_branches;

	public:
		CBranchListModel (QObject* _parent = 0);
		~CBranchListModel ();

		/// Load the list of local branches of specified git repository
		void loadFromGit (const QString& _repo_path);

		/// Check for existance of at least one branch in git repository
		bool empty () const;

	public:
		/// @name Implementation of QAbstractItemModel interface
		/** @{*/
		int rowCount (const QModelIndex& _parent = QModelIndex ()) const;
		int columnCount (const QModelIndex& _parent = QModelIndex ()) const;

		QVariant data (const QModelIndex& _index, int _role = Qt::DisplayRole) const;

		virtual QModelIndex index (int _row, int _column, const QModelIndex& _parent = QModelIndex ()) const;
		virtual QModelIndex parent (const QModelIndex& _child) const;
		/** @}*/
	};
}

#endif // __QGITREPOVIEWER_CBRANCHMODEL_H

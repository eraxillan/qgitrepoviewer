/**
 * @file
 * @brief Table data model for representing commits of git repository interface
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (C) SpectrumSoft, 2010-2013
 */
#ifndef __QGITREPOVIEWER_CCOMMITMODEL_H
#define __QGITREPOVIEWER_CCOMMITMODEL_H

#include <QStringList>
#include <QAbstractTableModel>

struct git_repository;

namespace QGitRepoViewer
{
	/// Table data model for representing commits of git repository
	class CCommitTableModel : public QAbstractTableModel
	{
		Q_OBJECT

		/// The list of SHA-1 commit hashes
		QStringList m_commits;

		/// Pointer to the git repository object
		git_repository* m_repo;

	public:
		/// Table columns: commit short log, commit author name and email, commit date
		enum { _ShortLogColumn = 0, _AuthorColumn = 1, _DateColumn, _ColumntCount };

		CCommitTableModel (QObject* _parent = 0);
		~CCommitTableModel ();

		/// Setup the git repository to view
		void setGitRepo (const QString& _repo_path);

		/// Return row index of commit with specified SHA-1 id or -1 if it was not found
		int commitIndex (const QString _commit_id) const;

		/// Load the commit list of specified git repository local branch
		void setCommitList (const QString& _branch_name);

		/// Check whether at least one commit was found
		bool empty () const;

	public:
		/**
		 * @name Implementation of QAbstractItemModel interface
		 */
		/** @{*/
		int rowCount (const QModelIndex& _parent = QModelIndex ()) const;
		int columnCount (const QModelIndex& _parent = QModelIndex ()) const;

		QVariant data (const QModelIndex& _index, int _role = Qt::DisplayRole) const;
		QVariant headerData (int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const;

		QModelIndexList	match (const QModelIndex& _start, int _role, const QVariant& _value, int _hits = 1,
							   Qt::MatchFlags _flags = Qt::MatchStartsWith | Qt::MatchWrap) const;
		/** @}*/
	};
}

#endif // __QGITREPOVIEWER_CCOMMITMODEL_H

/**
 * @file
 * @brief Main window of qgitrepoviewer declaration
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (C) SpectrumSoft, 2010-2013
 */

#ifndef __CMAINWINDOW_H
#define __CMAINWINDOW_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>

#include "ui_CMainWindow.h"

struct git_repository;

namespace QGitRepoViewer
{
	class CCommitTableModel;
	class CBranchListModel;

	/**
	 * @brief Main window of QGitRepoViewer application: contain branch and commit view controls
	 */
	class CMainWindow : public QMainWindow
	{
		Q_OBJECT

		/**
		  * @brief Qt GUI object
		  */
		Ui::CMainWindow m_ui;

		/**
		  * @brief Custom data model for list of local branches of git repository
		  */
		CBranchListModel* m_branch_model;

		/**
		  * @brief Custom data model for list of commits of git repository
		  */
		CCommitTableModel* m_commit_model;

		/**
		  * @brief The path to the git repository (dir with .git folder or any it's subfolder)
		  */
		QString m_repo_path;

		/**
		 * @brief Return the SHA-1 id of selected in list commit
		 */
		QString selectedCommitId () const;

		/**
		  * @brief Save some application settings on window hide event
		  */
		void hideEvent (QResizeEvent* _event);

		/**
		  * @brief Save some application settings on window hide event
		  */
		void showEvent (QShowEvent* _ev);

	private	Q_SLOTS:
		/**
		  * @brief Selected local branch was changed
		  */
		void aboutBranchSelected (const QString& _item);

		/**
		 * @brief Selected commit was changed
		 */
		void aboutCommitSelected (const QModelIndex& _current, const QModelIndex& _previous);

		/**
		  * @brief Filter search column index was changed
		  */
		void aboutFilterChanged (int _column_idx);

		/**
		 * @brief Open repository
		 */
		void on_action_open_repo_triggered ();

		/**
		 * @brief Quit from the program
		 */
		void on_action_quit_triggered ();

	public:
		/**
		  * @brief Constructs main window object with optional parent
		  *
		  * @param _parent Parent widget
		  */
		CMainWindow (QWidget* _parent = 0);

		~CMainWindow();
	};
}

#endif // __CMAINWINDOW_H

/**
 * @file
 * @brief Main window of qgitrepoviewer implementation
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (С) SpectrumSoft, 2010-2013
 */

#include "CMainWindow.h"

#include "CCommitModel.h"
#include "CBranchModel.h"

#include <QDir>
#include <QFileDialog>
#include <QSettings>

using namespace QGitRepoViewer;

#define GEOMETRY_KEY "ui/geometry"
#define GIT_REPO_KEY "repos/last"
#define BRANCH_KEY "ui/branch-index"
#define COMMIT_KEY "ui/commit-index"
#define FILTER_KEY "ui/filter-index"

/////////////////////////////////////////////////////////////////////////////////////////////////////

CMainWindow::CMainWindow (QWidget* _parent):
	QMainWindow (_parent),
	m_branch_model (NULL),
	m_commit_model (NULL)
{
	//
	// Initialize window GUI from Qt *.ui file
	//
	m_ui.setupUi (this);

	//
	// Connect git repository local branches model to appropriate combobox
	//
	m_branch_model = new CBranchListModel (this);
	m_ui.branch_list->setModel (m_branch_model);

	//
	// Connect git repository commits model to appropriate tableview
	//
	m_commit_model = new CCommitTableModel (this);
	m_ui.commit_list->setModel (m_commit_model);

	//
	// Handle user click on commit: show commit hash in appropriate text field
	//
	connect (m_ui.commit_list->selectionModel (), SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
			 this, SLOT (aboutCommitSelected (const QModelIndex&, const QModelIndex&)));

	//
	// Connect search widget to commits table (can search by brief commit description/author/date)
	//
	m_ui.commit_search->setView (m_ui.commit_list);

	//
	// Restore window geometry (position on the screen + size) from settings
	//
	QSettings settings ("SpectrumSoft", "qpiket");
	QVariant sizes_as_var = settings.value (GEOMETRY_KEY);
	if (! sizes_as_var.isNull () && sizes_as_var.canConvert <QByteArray> ())
		restoreGeometry (sizes_as_var.toByteArray ());
	else
		resize (800, 600);
}

CMainWindow::~CMainWindow ()
{
	//
	// Disconnect commit search widget from commit table
	//
	m_ui.commit_search->setView (NULL);

	//
	// Save last discovered git repository path if one presents in settings
	//
	QSettings settings ("SpectrumSoft", "qpiket");
	settings.setValue (GIT_REPO_KEY, m_repo_path);

	//
	// Save last user-selected branch of git repository
	//
	settings.setValue (BRANCH_KEY, m_ui.branch_list->currentIndex ());

	//
	// Save last user-selected commit
	//
	settings.setValue (COMMIT_KEY, m_ui.commit_list->currentIndex ().row ());

	//
	// Save last user-selected filter index
	//
	settings.setValue (FILTER_KEY, m_ui.filter_criteria->currentIndex ());
}

void
CMainWindow::hideEvent (QResizeEvent* _event)
{
	QMainWindow::resizeEvent (_event);

	//
	// Save dialog geometry (posion on the screen and size) in application settings
	//
	QSettings settings ("SpectrumSoft", "qpiket");
	settings.setValue (GEOMETRY_KEY, saveGeometry ());
}

void
CMainWindow::showEvent (QShowEvent* _ev)
{
	QMainWindow::showEvent (_ev);

	//
	// Restore last discovered git repository path if one presents in settings
	//
	QSettings settings ("SpectrumSoft", "qpiket");
	QVariant repo_path_var = settings.value (GIT_REPO_KEY);
	if (!repo_path_var.isNull () && repo_path_var.canConvert <QString> ())
	{
		QString path = repo_path_var.toString ();
		if (QDir (path).exists ())
			m_repo_path = path;
	}

	if (! m_repo_path.isEmpty ())
	{
		//
		// Connect custom table model to git repository using its path
		//
		m_commit_model->setGitRepo (m_repo_path);

		//
		// Load the list of git repository branches
		//
		m_branch_model->loadFromGit (m_repo_path);

		setWindowTitle ("QGitRepoViewer: " + m_repo_path);

		//
		// Restore last user-selected branch
		//
		QVariant branch_as_var = settings.value (BRANCH_KEY);
		if (! branch_as_var.isNull () && branch_as_var.canConvert <int> ())
		{
			bool ok = false;
			int branch_index = branch_as_var.toInt (& ok);
			if (ok && (branch_index >= 0) && (branch_index < m_ui.branch_list->count ()))
				m_ui.branch_list->setCurrentIndex (branch_index);
		}

		//
		// Restore last user-selected commit
		//
		QVariant commit_idx_var = settings.value (COMMIT_KEY);
		if (!commit_idx_var.isNull () && commit_idx_var.canConvert <int> ())
		{
			bool ok = false;
			int commit_index = commit_idx_var.toInt (& ok);
			if (ok && (commit_index >= 0) && (commit_index < m_commit_model->rowCount ()))
				m_ui.commit_list->setCurrentIndex (m_commit_model->index (commit_index, 0));
		}

		//
		// Restore last user-selected filter index
		//
		QVariant filter_idx_var = settings.value (FILTER_KEY, m_ui.filter_criteria->currentIndex ());
		if (! filter_idx_var.isNull () && filter_idx_var.canConvert <int> ())
		{
			bool ok = false;
			int filter_index = filter_idx_var.toInt (& ok);
			if (ok && (filter_index >= 0) && (filter_index < m_ui.filter_criteria->count ()))
				m_ui.filter_criteria->setCurrentIndex (filter_index);
		}
	}
}

void
CMainWindow::aboutBranchSelected (const QString& _item)
{
	//
	// Obtain new selected branch name
	//
	QString current_branch = _item;
	if (current_branch.isEmpty () || current_branch.size () < 3)
		return;
	if (current_branch.at (0) == '*')
		current_branch.remove (0, 2);
	else
		current_branch = current_branch.trimmed ();

	//
	// Fill the table with new branch commit list and select first of them
	//
	m_commit_model->setCommitList (current_branch);
	if (! m_commit_model->empty ())
		m_ui.commit_list->selectRow (0);

	//
	// Setup default column sizes as 60%, 20%, 20% (because such sizes looks fine)
	//
	int table_width = m_ui.commit_list->width () - 15;
	m_ui.commit_list->setColumnWidth (CCommitTableModel::_ShortLogColumn, (table_width) * 0.6);
	m_ui.commit_list->setColumnWidth (CCommitTableModel::_AuthorColumn, (table_width) * 0.2);
	m_ui.commit_list->setColumnWidth (CCommitTableModel::_DateColumn, (table_width) * 0.2);
}

void
CMainWindow::aboutCommitSelected (const QModelIndex& _current, const QModelIndex& _previous)
{
	Q_UNUSED (_previous);

	if (_current.isValid ())
		m_ui.commit_hash->setText (selectedCommitId ());
}

void
CMainWindow::aboutFilterChanged (int _column_idx)
{
	m_ui.commit_search->setSearchColumn (_column_idx);
}

QString
CMainWindow::selectedCommitId () const
{
	//
	// Obtain the ID of user selected commit
	//
	QString result;
	if (m_ui.commit_list->selectionModel ()->hasSelection ())
	{
		QModelIndexList commits_idx = m_ui.commit_list->selectionModel ()->selectedIndexes ();
		if (commits_idx.size () >= 1)
			result = m_commit_model->data (commits_idx.first (), Qt::UserRole).toString ();
	}

	return result;
}

void
QGitRepoViewer::CMainWindow::on_action_open_repo_triggered ()
{
	//
	// Let user select the directory with git repository (or subdirectory of it) in standart way
	//
	QString repo_dir = QFileDialog::getExistingDirectory (this, "Select directory with git repository");
	if (! repo_dir.isEmpty ())
	{
		m_repo_path = repo_dir;

		//
		// Connect custom table model to git repository using its path
		//
		m_commit_model->setGitRepo (m_repo_path);
		setWindowTitle ("QGitRepoViewer: " + m_repo_path);

		//
		// Load the list of git repository branches
		//
		m_branch_model->loadFromGit (m_repo_path);
		if (! m_branch_model->empty ())
			m_ui.branch_list->setCurrentIndex (0);
	}
}

void QGitRepoViewer::CMainWindow::on_action_exit_triggered()
{
	//
	// Close the main application window and quit
	//
	close ();
}

void QGitRepoViewer::CMainWindow::on_action_create_branch_triggered()
{
	//
}

void QGitRepoViewer::CMainWindow::on_action_delete_branch_triggered()
{
	//
}

void QGitRepoViewer::CMainWindow::on_action_move_branch_triggered()
{
	//
}

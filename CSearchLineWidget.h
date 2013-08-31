/**
 * @file
 * @brief View search widget interface
 * @author Eugene Turyansky <eutm@spectrum-soft.ru>
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 *
 * (С) SpectrumSoft, 2010-2013
 */

#ifndef __QGITREPOVIEWER_CSEARCHLINEWIDGET_H
#define __QGITREPOVIEWER_CSEARCHLINEWIDGET_H

#include <QWidget>
#include <QAbstractItemView>

namespace Ui
{
	class CSearchLineWidget;
}

namespace QGitRepoViewer
{
	/// Widget for searching any text in specified view columns
	class CSearchLineWidget : public QWidget
	{
		Q_OBJECT

		/// Qt GUI object
		QScopedPointer<Ui::CSearchLineWidget> m_ui;

		/// View for searching in
		QAbstractItemView* m_view;

		/// String that we should find
		QString m_pattern;

		/// List of currently found view items which matched the pattern
		QModelIndexList m_matched_nodes;

		/// Currenly selected item index from found ones
		int m_matched_node;

		/// Index of column to search in (first by default, i.e. 0)
		int m_column_idx;

	private slots:
		/// Find all matches of pattern in the view column with first one selection
		void findMatched (const QString& _pattern);

		/// Find all matches of pattern in the view column without first one selection
		void findMatched ();

		/// Find previous match of pattern in the view column
		void findPrev ();

		/// Find next match of pattern in the view column
		void findNext ();

	public:
		explicit CSearchLineWidget (QWidget* _parent = NULL);
		~CSearchLineWidget ();

		/// Setup the view to search in
		/// @attention Must be called before any others functions
		void setView (QAbstractItemView* _view = NULL);

		/// Setup the view column to search in
		void setSearchColumn (int _idx);
	};
}

#endif // __QGITREPOVIEWER_CSEARCHLINEWIDGET_H

#ifndef QLZMAOPTIONDIALOG_H
#define QLZMAOPTIONDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>

class QLineEdit; //file
class QPushButton; //file choose
class QComboBox; //level, method, dict
class QLable;

class QLzmaOptionDialogPrivate;
class QLzmaOptionDialog : public QDialog
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(QLzmaOptionDialog)
public:
	explicit QLzmaOptionDialog(QWidget *parent = 0);

	void retranslateUi();

signals:

public slots:

private:
	QLzmaOptionDialogPrivate *d_ptr;
};

#endif // QLZMAOPTIONDIALOG_H

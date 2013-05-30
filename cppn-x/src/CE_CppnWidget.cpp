/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "CE_CppnWidget.h"

#include <QtGui>

#include <math.h>
#include <cmath>


static const double Pi = 3.14159265358979323846264338327950288419717;

class QSlider;

//! [0]
GraphWidget::GraphWidget(QWidget *parent)
    : QGraphicsView(parent), timerId(0), edgeId(-1), sliderValue(0), selectedEdge(0)
{
	cppn = shared_ptr<NEAT::GeneticIndividual>();
	cppn_phen = shared_ptr<NEAT::FastNetwork<double> >();

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-400, -600, 800, 1200);
    connect(scene, SIGNAL(selectionChanged()), this, SLOT(selectEdge()));
    setScene(scene);

    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(qreal(0.8), qreal(0.8));
    setMinimumSize(400, 400);
//! [0]

}
//! [1]




void GraphWidget::updateNodes(){
	double res =2;
	int updates = 7;
	double scale=1;
	double dscale=1.4;
	for(unsigned long x=0; x<width; x++){
		for(unsigned long y=0; y<height; y++){
			double xv = (double(x)/(double(width)/res) - (res/2))*scale;
			double yv = (double(y)/(double(height)/res) - (res/2))*scale;

			cppn_phen->reinitialize();
			cppn_phen->setValue(io_map[INPUT_X], xv);
			cppn_phen->setValue(io_map[INPUT_Y], yv);
			cppn_phen->setValue(io_map[INPUT_BIAS], scale);
			cppn_phen->setValue(io_map[INPUT_D], std::sqrt(float(xv*xv+yv*yv))*dscale);
			cppn_phen->update(updates);

			for(int i=0; i<nodes.size(); i++){
				char grey = char(std::min(std::abs(cppn_phen->getValue(nodes[i]->getName())), 1.0)*255);
				nodes[i]->setPixel(x, y, grey, grey, grey );
			}
		}
	}

	for(int i=0; i<nodes.size(); i++){
		nodes[i]->update();
	}
}

void GraphWidget::positionNodes(){
	std::vector< std::vector <Node*> > layers;
	layers.push_back(std::vector <Node*>());
	std::vector <Node*> notPlaced = nodes;
	std::vector <Node*> nextNotPlaced;
	std::map <Node*, int> incommingEdges;

	for(int i=0; i<nodes.size(); i++){
		incommingEdges[nodes[i]]= nodes[i]->incomingEdges().size();
		//std::cout << nodes[i]->incomingEdges().size() << std::endl;
	}

	while(notPlaced.size()>0){
		nextNotPlaced.clear();
		for(int i=0; i<notPlaced.size(); i++){
//			std::cout << i <<" Incoming Edges: " << incommingEdges[notPlaced[i]] << std::endl;
			if(incommingEdges[notPlaced[i]] <= 0){
				layers.back().push_back(notPlaced[i]);
			} else {
				nextNotPlaced.push_back(notPlaced[i]);
			}
			//notPlaced[i]->
		}

		for(int i=0; i<layers.back().size(); i++){
			QList<Edge *> outgoingEdges = layers.back()[i]->outgoingEdges();
			foreach(Edge* outgoing_edge, outgoingEdges){
				incommingEdges.find(outgoing_edge->destNode())->second = incommingEdges[outgoing_edge->destNode()]-1;
			}
		}

		layers.push_back(std::vector <Node*>());

//		std::cout << "Next not Placed: "  << nextNotPlaced.size() << std::endl;
		notPlaced = nextNotPlaced;
//		std::cout << "Not Placed: "  << notPlaced.size() << std::endl;
	}



	int k =1;
	QPointF position1;
	QPointF position2;
	Node* edge1;
	qreal distanceEdge1;
	qreal distanceEdge2;

	for(int i=0; i< layers.size(); i++){
		for(int j=0; j< layers[i].size(); j++){
			if(k%2 != 0){
				position1 = QPointF(300 * std::cos(float(((Pi/(nodes.size()))*k)+ Pi/2)), 300 * std::sin(float(((Pi/(nodes.size()))*k)+ Pi/2)) );
				position2 = QPointF(-300 * std::cos(float(((Pi/(nodes.size()))*k)+ Pi/2)), 300 * std::sin(float(((Pi/(nodes.size()))*k)+ Pi/2)) );
			}
			qreal distance1 = 0;
			qreal distance2 = 0;
			foreach(Edge* edge, layers[i][j]->incomingEdges()){
				distance1 += QLineF(edge->sourceNode()->pos(), position1).length();
				distance2 += QLineF(edge->sourceNode()->pos(), position2).length();
			}


			if(k%2 == 0){
				if(distanceEdge1 + distance2 < distanceEdge2 +distance1){
					edge1->setPos(position1);
					layers[i][j]->setPos(position2);
				} else {
					edge1->setPos(position2);
					layers[i][j]->setPos(position1);
				}
				edge1 = NULL;
			}else {
				edge1 = layers[i][j];
				distanceEdge1 = distance1;
				distanceEdge2 = distance2;
			}
			k++;
		}
	}

	if(edge1 != NULL){
		if(distanceEdge1 < distanceEdge2){
			edge1->setPos(position1);
		} else {
			edge1->setPos(position2);
		}
	}

}


void GraphWidget::load(std::string filename)
{
	try{

		std::map<int, Node*> nodeMap;
		cppn = CppnParser::parse(filename, io_map);
		cppn_phen = shared_ptr<NEAT::FastNetwork<double> >(new NEAT::FastNetwork<double>(cppn->spawnFastPhenotypeStack<double>()));

		for(int i=0; i< cppn->getNodesCount();i++){
			std::string name = cppn->getNode(i)->getName();
			Node *node = new Node(this, i, name, width, height);
			nodeMap[cppn->getNode(i)->getID()] = node;
			nodes.push_back(node);
			scene()->addItem(node);
			node->setPos(0, -300 + 30*i);

		}


		//std::cout << cppn_phen->getLinkCount() << std::endl;

		for(int i=0; i< cppn_phen->getLinkCount();i++){
			//std::cout << "From: " << nodeMap[cppn->getLink(i)->getFromNodeID()] << " to: " <<  nodeMap[cppn->getLink(i)->getToNodeID()]<< std::endl;
			Edge* edge = new Edge(this, nodeMap[cppn->getLink(i)->getFromNodeID()], nodeMap[cppn->getLink(i)->getToNodeID()], i, cppn->getLink(i)->getWeight());
			scene()->addItem(edge);
		}

		positionNodes();
		updateNodes();
	} catch(std::exception& e){
		QString message(("Error reading file: " + filename + "\n" + std::string( e.what() )).c_str());
		QMessageBox::information(this, tr("Unable to open file"), message);
	}
//	catch (std::ifstream::failure e) {
//		//std::cout << e.what();
//		//cppn = shared_ptr<NEAT::GeneticIndividual>();
//		QString message(("Error reading file: " + filename + "\n" + std::string(e.what())).c_str());
//		QMessageBox::information(this, tr("Unable to open file"), message);
//	}
//	std::cout << "It did not crash yet" << std::endl;

}


//! [2]
void GraphWidget::itemMoved()
{
//    if (!timerId)
//        timerId = startTimer(1000 / 25);
}
//! [2]

//! [3]
void GraphWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:

        break;
    case Qt::Key_Down:

        break;
    case Qt::Key_Left:

        break;
    case Qt::Key_Right:

        break;
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Space:
    case Qt::Key_Enter:
//        shuffle();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}
//! [3]

//! [4]
void GraphWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if(sliderValue <101){
    	setValue(sliderValue+1);
    }else{
    	killTimer(timerId);
    	timerId=0;
    }

//    std::cout <<  "Timer: "<< sliderValue << std::endl;


//    QList<Node *> nodes;
//    foreach (QGraphicsItem *item, scene()->items()) {
//        if (Node *node = qgraphicsitem_cast<Node *>(item))
//            nodes << node;
//    }
//
////    foreach (Node *node, nodes)
////        node->calculateForces();
//
//    bool itemsMoved = false;
//    foreach (Node *node, nodes) {
//        if (node->advance())
//            itemsMoved = true;
//    }
//
//    if (!itemsMoved) {
//        killTimer(timerId);
//        timerId = 0;
//    }
}
//! [4]

//! [5]
void GraphWidget::wheelEvent(QWheelEvent *event)
{
    scaleView(std::pow((double)2, event->delta() / 240.0));
}
//! [5]

//! [6]
void GraphWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);

    // Shadow
    QRectF sceneRect = this->sceneRect();
    QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    if (rightShadow.intersects(rect) || rightShadow.contains(rect))
	painter->fillRect(rightShadow, Qt::darkGray);
    if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
	painter->fillRect(bottomShadow, Qt::darkGray);

    // Fill
    QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::white);
    //gradient.setColorAt(1, Qt::lightGray);
    painter->fillRect(rect.intersect(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);

#if !defined(Q_OS_SYMBIAN) && !defined(Q_WS_MAEMO_5)
    // Text
//    QRectF textRect(sceneRect.left() + 4, sceneRect.top() + 4, sceneRect.width() - 4, sceneRect.height() - 4);
//    QString message(tr("Click and drag the nodes around, and zoom with the mouse wheel or the '+' and '-' keys"));
//
//    QFont font = painter->font();
//    font.setBold(true);
//    font.setPointSize(14);
//    painter->setFont(font);
//    painter->setPen(Qt::lightGray);
//    painter->drawText(textRect.translated(2, 2), message);
//    painter->setPen(Qt::black);
//    painter->drawText(textRect, message);
#endif
}
//! [6]

//! [7]
void GraphWidget::scaleView(qreal scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
}
//! [7]

//void GraphWidget::shuffle()
//{
//    foreach (QGraphicsItem *item, scene()->items()) {
//        if (qgraphicsitem_cast<Node *>(item))
//            item->setPos(-150 + qrand() % 300, -150 + qrand() % 300);
//    }
//}

void GraphWidget::zoomIn()
{
    scaleView(qreal(1.2));
}

void GraphWidget::zoomOut()
{
    scaleView(1 / qreal(1.2));
}

void GraphWidget::edgeSelected(int id, Edge* selectedEdge){
//	std::cout << id << std::endl;

}

void GraphWidget::setValue(int value){

	if(sliderValue!=value){
		double newWeight = (float(value))*0.06-3;
		//std::cout << "Set Value: "<< newWeight << std::endl;
		emit sliderValueChangedF(newWeight);
	}

	sliderValue=value;
}

void GraphWidget::setValueF(double weight){
//	std::cout <<  "Set weight f: "<< weight << std::endl;
	int value = round(((weight+3)/6)*100);
//	std::cout <<  "Set Value f: "<< value << std::endl;
	sliderValue=value;
	emit sliderValueChanged(value);

	if(edgeId >=0 && selectedEdge){
		selectedEdge->setWeight(weight);
		selectedEdge->update();
		cppn_phen->getLink(edgeId)->weight=weight;
		updateNodes();
	}
}

void GraphWidget::resetWeight(){
	if(edgeId >=0 && selectedEdge){
		emit sliderValueChangedF(selectedEdge->getOriginalWeight());
	}
}

void GraphWidget::scanWeight(){
	if(!timerId){
		timerId = startTimer(100);
		setValue(0);
	} else {
		killTimer(timerId);
		timerId=0;
	}
}


void GraphWidget::selectEdge(){
	if(scene()->selectedItems().count() == 0){
		edgeId = -1;
		selectedEdge = NULL;
		killTimer(timerId);
		return;
	}
	selectedEdge = qgraphicsitem_cast<Edge*> ( scene()->selectedItems().front());
	if(selectedEdge){
		edgeId=selectedEdge->getId();
		emit sliderValueChangedF(cppn_phen->getLink(edgeId)->weight);
	}
}

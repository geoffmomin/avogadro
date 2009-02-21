/**********************************************************************
  DrawCommandTest - unit testing for the the draw QUndoCommands used by the drawtool

  Copyright (C) 2009 Tim Vandermeersch

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.openmolecules.net/>

  Avogadro is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#include <QtTest>
#include <Eigen/Core>
#include <avogadro/molecule.h>
#include <avogadro/atom.h>
#include <avogadro/bond.h>

#include "../src/tools/drawcommand.cpp"
/*
  AddAtomDrawCommand(Molecule *molecule, const Eigen::Vector3d& pos,
      unsigned int element, int adjustValence);
  AddAtomDrawCommand(Molecule *molecule, Atom *atom, int adjustValence);
  DeleteAtomDrawCommand(Molecule *molecule, int index, int adjustValence);
  AddBondDrawCommand(Molecule *molecule, Atom *beginAtom, Atom *endAtom, unsigned int order, int adjustValence);
  AddBondDrawCommand(Molecule *molecule, Bond *bond, int adjustValence);
  DeleteBondDrawCommand(Molecule *molecule, int index, int adjustValence);
  ChangeElementDrawCommand(Molecule *molecule, Atom *atom, 
      unsigned int element, int adjustValence);
  ChangeBondOrderDrawCommand(Molecule *molecule, Bond *bond,
      unsigned int bondOrder, int adjustValence);
InsertFragmentCommand(Molecule *molecule, Molecule &generatedMolecule); 
*/
using Avogadro::AdjustHydrogensPreCommand;
using Avogadro::AdjustHydrogensPostCommand;
using Avogadro::AddAtomDrawCommand;
using Avogadro::DeleteAtomDrawCommand;
using Avogadro::AddBondDrawCommand;
using Avogadro::DeleteBondDrawCommand;
using Avogadro::ChangeElementDrawCommand;
using Avogadro::ChangeBondOrderDrawCommand;

using Avogadro::Molecule;
using Avogadro::Atom;
using Avogadro::Bond;

using Eigen::Vector3d;

class DrawCommandTest : public QObject
{
  Q_OBJECT

  private:
    QUndoStack *m_undoStack; /// QUndoStack object for use by the test class.
    Molecule   *m_molecule; /// Molecule object for use by the test class.

  private:
    void debugMolecule();

    void loopUndoRedo();

  private slots:
    /**
     * Test AdjustHydrogensPreCommand
     */
    void AdjustHydrogensPreCommand_methane();
    /**
     * Test AdjustHydrogensPreCommand
     */
    void AdjustHydrogensPostCommand_methane();


    /** 
     * Test AddAtomDrawCommand(molecule, pos, element, adj=1)
     */
    void AddAtom_methane();
    /** 
     * Test AddAtomDrawCommand(molecule, atom, adj=1)
     */
    void AddAtom_ammonia();
    /** 
     * Test AddAtomDrawCommand(molecule, pos, element, adj=0)
     * Test AddAtomDrawCommand(molecule, atom, adj=0)
     */
    void AddAtom_water();

 
    /**
     * Test ChangeElementDrawCommand(molecule, atom, oldElement, adj=1);
     */
    void ChangeElement_ethane();
    /**
     * Test ChangeElementDrawCommand(molecule, atom, oldElement, adj=0);
     */
    void ChangeElement_carbon();

    /** 
     * Test AddAtomDrawCommand(molecule, index, adj=1)
     */ 
    void DeleteAtom_methane();
    /** 
     * Test AddAtomDrawCommand(molecule, index, adj=0)
     */ 
    void DeleteAtom_carbon();


    /**
     * Test AddBondDrawCommand(molecule, beginAtom, endAtom, order, adj=1)
     */
    void AddBond_ethane();
    /**
     * Test AddBondDrawCommand(molecule, bond, adj=1)
     */
    void AddBond_ethane2();

    /**
     * Test ChangeBondOrderDrawCommand(molecule, index, adj=1)
     */
    void ChangeBondOrder_ethane();
    void ChangeBondOrder_ethene();
 
   
    /**
     * Test DeleteBondDrawCommand(molecule, index, adj=1)
     */
    void DeleteBond_ethane();
    
    // 2x AddAtom + AddBond
    /*
    void AddAtomDrawCommand_ethane();
    void AddAtomDrawCommand_methanol();

    void AddAtom_AddBond_DeleteAtom();
    void AddAtom_ChangeElement_DeleteAtom();
*/
    
  
    
    // Crashers...
    void crash1();
    void crash2();
    void crash3();

    void DeleteAtom_ethane();

    /**
     * Called before the first test function is executed.
     */
    void initTestCase();

    /**
     * Called after the last test function is executed.
     */
    void cleanupTestCase();

    /**
     * Called before each test function is executed.
     */
    void init();

    /**
     * Called after every test function.
     */
    void cleanup();


};

void DrawCommandTest::initTestCase()
{
}

void DrawCommandTest::cleanupTestCase()
{
}

void DrawCommandTest::init()
{
  m_molecule = new Molecule;
  m_undoStack = new QUndoStack;
}

void DrawCommandTest::cleanup()
{
  delete m_molecule;
  m_molecule = 0;
  delete m_undoStack;
  m_undoStack = 0;
}
    
void DrawCommandTest::debugMolecule()
{
  qDebug() << "Atoms: index, id, atomicNumber";
  foreach (Atom *atom, m_molecule->atoms())
    qDebug() << "  index=" << atom->index() << ", id=" << atom->id() << ", atomicNumber=" << atom->atomicNumber()
             << ", valence=" << atom->valence();
  
  qDebug() << "Bonds: index, id, begin, end";
  foreach (Bond *bond, m_molecule->bonds())
    qDebug() << "  index=" << bond->index() << ", id=" << bond->id() << ", begin=" 
             << bond->beginAtomId() << ", end=" << bond->endAtomId();
}


void DrawCommandTest::loopUndoRedo()
{
  // save some variables to check after undo/redo loop
  unsigned int numAtoms = m_molecule->numAtoms();
  // save the index
  int cmdIndex = m_undoStack->index();

  // save all atom & bond ids
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  for (int i = 0; i < 10; ++i) {
    m_undoStack->setIndex(0); // undo all
    QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
    m_undoStack->setIndex(cmdIndex); // redo all
    QCOMPARE(m_molecule->numAtoms(), numAtoms);
  }

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));


}

void DrawCommandTest::AddAtom_methane()
{
  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(4)->atomicNumber(), 1);

  loopUndoRedo();  

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(4)->atomicNumber(), 1);
}

void DrawCommandTest::AddAtom_ammonia()
{
  Atom *atom = m_molecule->addAtom();
  atom->setAtomicNumber(7);

  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 3);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 3);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 7);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);

  loopUndoRedo();  

  // check if the hydrogens still have the same ids
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 3);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 3);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 7);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
}

void DrawCommandTest::AddAtom_water()
{
  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 8, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 3);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 2);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 2);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 8);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
 
  loopUndoRedo();  

  // check if the hydrogens still have the same ids
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 3);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 2);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 2);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 8);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
}

/*
void DrawCommandTest::AddAtomDrawCommand_ethane()
{
  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);

  Atom *beginAtom = m_molecule->atomById(0); // C1
  Atom *endAtom = m_molecule->atomById(5); // C2

  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 1, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  
  loopUndoRedo();
}

void DrawCommandTest::AddAtomDrawCommand_methanol()
{
  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 8, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);

  Atom *beginAtom = m_molecule->atomById(0); // C
  Atom *endAtom = m_molecule->atomById(5); // O

  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 1, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 6);
  
  loopUndoRedo();
}

void DrawCommandTest::AddAtom_AddBond_DeleteAtom()
{
  unsigned int numAtoms = m_molecule->numAtoms();
  unsigned int numBonds = m_molecule->numBonds();
  
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  unsigned long beginAtomId = numAtoms;
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 5);
  QCOMPARE(m_molecule->atomById(beginAtomId)->valence(), 4.);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 4);

  // Add 2nd C --> CH4  CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  unsigned long endAtomId = numAtoms + 5;
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 10);
  QCOMPARE((int)m_molecule->atomById(endAtomId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 8);

  // Add C-C bond --> H3C--CH3
  Atom *beginAtom = m_molecule->atomById(beginAtomId); // C1
  Atom *endAtom = m_molecule->atomById(endAtomId); // C2
  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 1, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 8);
  QCOMPARE((int)m_molecule->atomById(beginAtomId)->valence(), 4);
  QCOMPARE((int)m_molecule->atomById(endAtomId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 7);

  Bond *bond = m_molecule->bond(beginAtom, endAtom);
  QVERIFY(bond);
  QCOMPARE(bond->id(), (unsigned long) numAtoms + 8);
 
  // Delete 1st C atom --> CH4
  m_undoStack->push( new DeleteAtomDrawCommand(m_molecule, beginAtom->index(), 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 5);
  QCOMPARE((int)m_molecule->atomById(endAtomId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 4);
 
  foreach (Bond *bond, m_molecule->bonds()) {
    QVERIFY( m_molecule->atomById(bond->beginAtomId()) ); // make sure C1 exists
    QVERIFY( m_molecule->atomById(bond->endAtomId()) ); // make sure C2 exists
  }

  // save the index
  int cmdIndex = m_undoStack->index();

  m_undoStack->setIndex(cmdIndex-1); // undo DeleteAtom
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE((int)m_molecule->atomById(beginAtomId)->valence(), 4);
  QCOMPARE((int)m_molecule->atomById(endAtomId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);
  //debugMolecule();
  qDebug() << m_molecule->bond(0, 5);
  
  QVERIFY( m_molecule->bond(beginAtomId, endAtomId) ); // make sure C1 and C2 are connected

  m_undoStack->setIndex(cmdIndex-2); // undo AddBond
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE((int)m_molecule->atomById(endAtomId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);
  m_undoStack->setIndex(cmdIndex-3); // undo AddAtom
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE((int)m_molecule->atomById(beginAtomId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  m_undoStack->setIndex(cmdIndex-4); // undo AddAtom
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
/////////// 
  bond->setOrder(2);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond, 1, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) offset + 6);

  bond->setOrder(3);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond, 2, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) offset + 4);

  m_undoStack->push( new DeleteBondDrawCommand(m_molecule, bond->index(), 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) offset + 10);
///////////
  loopUndoRedo();
}

void DrawCommandTest::AddAtom_ChangeElement_DeleteAtom()
{
  unsigned int numAtoms = m_molecule->numAtoms();
  unsigned int numBonds = m_molecule->numBonds();
  
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  unsigned long carbonId = numAtoms;
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 5);
  QCOMPARE((int)m_molecule->atomById(carbonId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 4);

  Atom *hydrogen = m_molecule->atom(numAtoms + 1);
  unsigned long hydrogenId = hydrogen->id();

  // Change H to C --> H3C--CH3
  hydrogen->setAtomicNumber(6);
  m_undoStack->push( new ChangeElementDrawCommand(m_molecule, hydrogen, 1, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 8);
  QCOMPARE((int)m_molecule->atomById(carbonId)->valence(), 4);
  QCOMPARE((int)m_molecule->atomById(hydrogenId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 7);

  // save the index
  int cmdIndex = m_undoStack->index();
  
  m_undoStack->setIndex(cmdIndex-1); // undo ChangeElement
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE((int)m_molecule->atomById(carbonId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  
  m_undoStack->setIndex(cmdIndex-2); // undo AddAtom
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
  
  m_undoStack->setIndex(cmdIndex); // redo all
  debugMolecule();
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) numAtoms + 8);
  QCOMPARE((int)m_molecule->atomById(carbonId)->valence(), 4);
  QCOMPARE((int)m_molecule->atomById(hydrogenId)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) numBonds + 7);


  loopUndoRedo();
}
*/

void DrawCommandTest::AdjustHydrogensPreCommand_methane()
{
  // Add C
  Atom *atom = m_molecule->addAtom();
  atom->setAtomicNumber(6);
  // Add hydrogens --> CH4
  m_molecule->addHydrogens(atom);

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);

  // run AdjustHydrogensPreCommand:
  // ::redo() saves the hydrogens' ids and removes the hydrogens
  // ::undo() adds the hydrogens again with the same ids
  QList<unsigned long> atomIds;
  atomIds.append(atom->id());
  m_undoStack->push( new AdjustHydrogensPreCommand(m_molecule, atomIds) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 1);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 0);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);

  // undo/redo 10 times
  int cmdIndex = m_undoStack->index();
  for (int i = 0; i < 10; ++i) {
    m_undoStack->setIndex(0); // undo 
    m_undoStack->setIndex(cmdIndex); // redo 
  }

  m_undoStack->setIndex(0); // undo 
  // check if the hydrogens still have the same ids
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());


}

void DrawCommandTest::AdjustHydrogensPostCommand_methane()
{
  // Add C
  Atom *atom = m_molecule->addAtom();
  atom->setAtomicNumber(6);

  // run AdjustHydrogensPostCommand:
  // ::redo() add hydrogens and save their ids, later calls will add the hydrogens with the same ids
  // ::undo() remove the hydrogens
  QList<unsigned long> atomIds;
  atomIds.append(atom->id());
  m_undoStack->push( new AdjustHydrogensPostCommand(m_molecule, atomIds) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);

  // undo/redo 10 times
  int cmdIndex = m_undoStack->index();
  for (int i = 0; i < 10; ++i) {
    m_undoStack->setIndex(0); // undo 
    m_undoStack->setIndex(cmdIndex); // redo 
  }

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);

  // check if the hydrogens still have the same ids
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

}


void DrawCommandTest::ChangeElement_ethane()
{
  // Add carbon --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);

  Atom *hydrogen = m_molecule->atom(1); // first hydrogen

  // Change H to C --> H3C--CH3
  hydrogen->setAtomicNumber(6);
  m_undoStack->push( new ChangeElementDrawCommand(m_molecule, hydrogen, 1, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE((int)m_molecule->atomById(1)->valence(), 4);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(5)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(6)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(7)->atomicNumber(), 1);

  loopUndoRedo();

  // check if the hydrogens still have the same ids
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE((int)m_molecule->atomById(1)->valence(), 4);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(5)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(6)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(7)->atomicNumber(), 1);
}

void DrawCommandTest::ChangeElement_carbon()
{
  // Add carbon
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 0) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 1);

  Atom *carbon = m_molecule->atom(0); // first hydrogen

  // Change C to O
  carbon->setAtomicNumber(8);
  m_undoStack->push( new ChangeElementDrawCommand(m_molecule, carbon, 6, 0) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 1);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 0);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 8);

  loopUndoRedo();

  // check if the hydrogens still have the same ids
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 1);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 0);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 8);
}

void DrawCommandTest::DeleteAtom_methane()
{
  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(4)->atomicNumber(), 1);
  
  m_undoStack->push( new DeleteAtomDrawCommand(m_molecule, 0, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
 
  loopUndoRedo();  

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
 
  m_undoStack->undo(); // undo DeleteAtom

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->index(), bond->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atomById(1)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atomById(4)->atomicNumber(), 1);
}

void DrawCommandTest::DeleteAtom_ethane()
{
  Atom *atom1 = m_molecule->addAtom();
  atom1->setAtomicNumber(6);
  Atom *atom2 = m_molecule->addAtom();
  atom2->setAtomicNumber(6);
  Bond *bond1 = m_molecule->addBond();
  bond1->setAtoms(atom1->id(), atom2->id(), 1);
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom1, 2) );
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom2, 2) );
  // AddBondDrawCommand(begin= 0 , end= 1 , adj= 2 )
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond1, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 4);
  
  m_undoStack->push( new DeleteAtomDrawCommand(m_molecule, 0, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);

  //debugMolecule();
 
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

}

void DrawCommandTest::DeleteAtom_carbon()
{
  // redo will be called automatically, the index will also be increased
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 0) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 1);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 0);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
  
  m_undoStack->push( new DeleteAtomDrawCommand(m_molecule, 0, 0) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 0);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
 
  loopUndoRedo();  

  m_undoStack->undo(); // undo DeleteAtom

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->index(), atom->id());

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 1);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 0);
  QCOMPARE((int)m_molecule->atomById(0)->valence(), 0);
  QCOMPARE(m_molecule->atomById(0)->atomicNumber(), 6);
}

void DrawCommandTest::AddBond_ethane()
{
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  // Add 2nd C --> CH4  CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);

  // Add C-C bond --> H3C--CH3
  Atom *beginAtom = m_molecule->atomById(0); // C1
  Atom *endAtom = m_molecule->atomById(5); // C2
  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 1, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE((int)beginAtom->valence(), 4);
  QCOMPARE((int)endAtom->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  Bond *bond = m_molecule->bond(beginAtom, endAtom);
  QVERIFY(bond);
  QCOMPARE(bond->id(), (unsigned long) 8);

  // save the atom & bond ids since they are no longer the same as indexes
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

  QCOMPARE((int)m_molecule->atom(0)->valence(), 4);
  QCOMPARE((int)m_molecule->atom(1)->valence(), 4);
  QCOMPARE(m_molecule->atom(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(5)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(6)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(7)->atomicNumber(), 1);
}

void DrawCommandTest::AddBond_ethane2()
{
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  // Add 2nd C --> CH4  CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);


  Bond *bond = m_molecule->addBond();
  Atom *beginAtom = m_molecule->atomById(0); // C1
  Atom *endAtom = m_molecule->atomById(5); // C2
  bond->setAtoms(0, 5, 1);

  // Add C-C bond --> H3C--CH3
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE((int)beginAtom->valence(), 4);
  QCOMPARE((int)endAtom->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  QVERIFY(bond);
  QCOMPARE(bond->id(), (unsigned long) 8);

  // save the atom & bond ids since they are no longer the same as indexes
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();

  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

  QCOMPARE((int)m_molecule->atom(0)->valence(), 4);
  QCOMPARE((int)m_molecule->atom(1)->valence(), 4);
  QCOMPARE(m_molecule->atom(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(5)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(6)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(7)->atomicNumber(), 1);
}

void DrawCommandTest::ChangeBondOrder_ethane()
{
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  // Add 2nd C --> CH4  CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);

  // Add C-C bond --> H3C--CH3
  Atom *beginAtom = m_molecule->atomById(0); // C1
  Atom *endAtom = m_molecule->atomById(5); // C2
  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 1, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE((int)beginAtom->valence(), 4);
  QCOMPARE((int)endAtom->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  Bond *bond = m_molecule->bond(beginAtom, endAtom);
  QVERIFY(bond);
  QCOMPARE(bond->id(), (unsigned long) 8);

  // save the atom & bond ids since they are no longer the same as indexes
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());
  
  //debugMolecule();

  bond->setOrder(2);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond, 1, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 6);

  m_undoStack->undo(); // undo ChangeBondOrder 1 -> 2
  
  //debugMolecule();

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

  m_undoStack->redo(); 
  loopUndoRedo();
  m_undoStack->undo(); // undo ChangeBondOrder 1 -> 2
  
  //debugMolecule();
  
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

  QCOMPARE((int)m_molecule->atom(0)->valence(), 4);
  QCOMPARE((int)m_molecule->atom(1)->valence(), 4);
  QCOMPARE(m_molecule->atom(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(5)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(6)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(7)->atomicNumber(), 1);
}

void DrawCommandTest::ChangeBondOrder_ethene()
{
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  // Add 2nd C --> CH4  CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);

  // Add C-C bond --> H3C--CH3
  Atom *beginAtom = m_molecule->atomById(0); // C1
  Atom *endAtom = m_molecule->atomById(5); // C2
  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 2, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 6);
  QCOMPARE((int)beginAtom->valence(), 3);
  QCOMPARE((int)endAtom->valence(), 3);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 5);

  Bond *bond = m_molecule->bond(beginAtom, endAtom);
  QVERIFY(bond);
  QCOMPARE(bond->id(), (unsigned long) 8);

  // save the atom & bond ids since they are no longer the same as indexes
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  //debugMolecule();

  bond->setOrder(3);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond, 2, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 3);

  m_undoStack->undo(); // undo ChangeBondOrder 2 -> 3
  //debugMolecule();

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

  m_undoStack->redo(); 
  loopUndoRedo();

  m_undoStack->undo(); // undo ChangeBondOrder 1 -> 2
  
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 6);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 5);

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QCOMPARE(atom->id(), atomIds.at(atom->index()));
  foreach (Bond *bond, m_molecule->bonds())
    QCOMPARE(bond->id(), bondIds.at(bond->index()));

  QCOMPARE((int)m_molecule->atom(0)->valence(), 3);
  QCOMPARE((int)m_molecule->atom(1)->valence(), 3);
  QCOMPARE(m_molecule->atom(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(5)->atomicNumber(), 1);
}

void DrawCommandTest::DeleteBond_ethane()
{
  // Add 1st C --> CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 5);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 4);
  // Add 2nd C --> CH4  CH4
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, Eigen::Vector3d::Zero(), 6, 1) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);

  //debugMolecule();
  // save the atom & bond ids since they are no longer the same as indexes
  QList<unsigned long> atomIds, bondIds;
  foreach (Atom *atom, m_molecule->atoms())
    atomIds.append(atom->id());
  foreach (Bond *bond, m_molecule->bonds())
    bondIds.append(bond->id());

  // Add C-C bond --> H3C--CH3
  Atom *beginAtom = m_molecule->atomById(0); // C1
  Atom *endAtom = m_molecule->atomById(5); // C2
  m_undoStack->push( new AddBondDrawCommand(m_molecule, beginAtom, endAtom, 1, 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE((int)beginAtom->valence(), 4);
  QCOMPARE((int)endAtom->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);

  Bond *bond = m_molecule->bond(beginAtom, endAtom);
  QVERIFY(bond);
  QCOMPARE(bond->id(), (unsigned long) 8);

  m_undoStack->push( new DeleteBondDrawCommand(m_molecule, bond->index(), 2) );
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE((int)beginAtom->valence(), 4);
  QCOMPARE((int)endAtom->valence(), 4);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();
  
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);
  m_undoStack->undo();
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 8);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 7);
  m_undoStack->undo();
  QCOMPARE(m_molecule->numAtoms(), (unsigned int) 10);
  QCOMPARE(m_molecule->numBonds(), (unsigned int) 8);
 
  //debugMolecule();

  // check if all atom & bond ids are still the same
  foreach (Atom *atom, m_molecule->atoms())
    QVERIFY(atomIds.contains(atom->id()));
  foreach (Bond *bond, m_molecule->bonds())
    QVERIFY(bondIds.contains(bond->id()));

  QCOMPARE((int)m_molecule->atom(0)->valence(), 4);
  QCOMPARE((int)m_molecule->atom(1)->valence(), 4);
  QCOMPARE(m_molecule->atom(0)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(1)->atomicNumber(), 6);
  QCOMPARE(m_molecule->atom(2)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(3)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(4)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(5)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(6)->atomicNumber(), 1);
  QCOMPARE(m_molecule->atom(7)->atomicNumber(), 1);
}
    
void DrawCommandTest::crash1()
{
  Atom *beginAtom = m_molecule->addAtom();
  beginAtom->setAtomicNumber(6);
  Atom *endAtom = m_molecule->addAtom();
  endAtom->setAtomicNumber(6);
  Bond *bond = m_molecule->addBond();
  bond->setAtoms(beginAtom->id(), endAtom->id(), 1);
  
  //AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, beginAtom, 2) );
  //AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, endAtom, 2) );
  //AddBondDrawCommand(begin= 0 , end= 1 , adj= 2 ) 
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond, 1) );
  //ChangeBondOrderDrawCommand(id= 0 , old= 1 , adj= 2 ) 
  bond->setOrder(2);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond, 1, 1) );

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();
}

void DrawCommandTest::crash2()
{
  Atom *atom1 = m_molecule->addAtom();
  atom1->setAtomicNumber(6);
  Atom *atom2 = m_molecule->addAtom();
  atom2->setAtomicNumber(6);
  Bond *bond1 = m_molecule->addBond();
  bond1->setAtoms(atom1->id(), atom2->id(), 1);
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom1, 2) );
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom2, 2) );
  // AddBondDrawCommand(begin= 0 , end= 1 , adj= 2 )
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond1, 1) );
 
  Atom *atom3 = m_molecule->addAtom();
  atom3->setAtomicNumber(6);
  Bond *bond2 = m_molecule->addBond();
  bond2->setAtoms(atom3->id(), atom2->id(), 1);
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom3, 2) );
  // AddBondDrawCommand(begin= 8 , end= 1 , adj= 2 ) 
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond2, 1) );
 
  Atom *atom4 = m_molecule->addAtom();
  atom4->setAtomicNumber(6);
  Bond *bond3 = m_molecule->addBond();
  bond3->setAtoms(atom2->id(), atom4->id(), 1);
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom4, 2) );
  // AddBondDrawCommand(begin= 1 , end= 15 , adj= 2 )
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond3, 1) );

  // ChangeBondOrderDrawCommand(id= 13 , old= 1 , adj= 2 )
  Bond *bond = m_molecule->bondById(13);
  bond->setOrder(2);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond, 1, 1) );

  // ChangeElementDrawCommand(id= 0 , old= 6 , adj= 2 )
  atom1->setAtomicNumber(8);
  m_undoStack->push( new ChangeElementDrawCommand(m_molecule, atom1, 6, 0) );
  // ChangeElementDrawCommand(id= 15 , old= 6 , adj= 2 ) 
  atom4->setAtomicNumber(8);
  m_undoStack->push( new ChangeElementDrawCommand(m_molecule, atom4, 6, 0) );
  
  // DeleteAtomDrawCommand(id= 8 , adj= 2 ) 
  m_undoStack->push( new DeleteAtomDrawCommand(m_molecule, atom3->index(), 2) );
  
  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();
}

void DrawCommandTest::crash3()
{
  Atom *atom1 = m_molecule->addAtom();
  atom1->setAtomicNumber(6);
  Atom *atom2 = m_molecule->addAtom();
  atom2->setAtomicNumber(6);
  Bond *bond1 = m_molecule->addBond();
  bond1->setAtoms(atom1->id(), atom2->id(), 1);
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom1, 2) );
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom2, 2) );
  // AddBondDrawCommand(begin= 0 , end= 1 , adj= 2 )
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond1, 1) );
  // ChangeBondOrderDrawCommand(id= 0 , old= 1 , adj= 2 ) 
  bond1->setOrder(2);
  m_undoStack->push( new ChangeBondOrderDrawCommand(m_molecule, bond1, 1, 1) );

  // DeleteAtomDrawCommand(id= 1 , adj= 2 ) 
  m_undoStack->push( new DeleteAtomDrawCommand(m_molecule, atom2->index(), 2) );

  Atom *atom3 = m_molecule->addAtom();
  atom3->setAtomicNumber(6);
  Bond *bond2 = m_molecule->addBond();
  bond2->setAtoms(atom1->id(), atom3->id(), 1);
  // AddAtomDrawCommand_ctor2(element= 6 , adj= 2 ) 
  m_undoStack->push( new AddAtomDrawCommand(m_molecule, atom3, 2) );
  // AddBondDrawCommand(begin= 14 , end= 0 , adj= 2 ) 
  m_undoStack->push( new AddBondDrawCommand(m_molecule, bond2, 1) );

  //debugMolecule();
  loopUndoRedo();
  //debugMolecule();

}


QTEST_MAIN(DrawCommandTest)

#include "moc_drawcommandtest.cxx"

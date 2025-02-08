using Autodesk.AutoCAD.Runtime;
using Autodesk.AutoCAD.ApplicationServices;
using Autodesk.AutoCAD.DatabaseServices;
using Autodesk.AutoCAD.Geometry;
using System;
using System.Collections.Generic;

public class SAFERCircuitDesigner
{
    // Constants for circuit elements
    private const double GRID_SIZE = 10.0;
    private const double TEXT_HEIGHT = 2.5;
    private const string CIRCUIT_LAYER = "SAFER_Circuit";
    private const string TEXT_LAYER = "SAFER_Text";

    [CommandMethod("DrawSAFERCircuit")]
    public void DrawCircuit()
    {
        Document doc = Application.DocumentManager.MdiActiveDocument;
        Database db = doc.Database;
        
        using (Transaction trans = db.TransactionManager.StartTransaction())
        {
            // Create layers
            CreateCircuitLayers(db);

            // Start drawing circuit elements
            BlockTable bt = trans.GetObject(db.BlockTableId, OpenMode.ForRead) as BlockTable;
            BlockTableRecord btr = trans.GetObject(bt[BlockTableRecord.ModelSpace], OpenMode.ForWrite) as BlockTableRecord;

            // Draw power rails
            DrawPowerRails(btr);
            
            // Draw sensor input stages
            DrawTemperatureSensor(btr, new Point3d(100, 100, 0));
            DrawPressureSensor(btr, new Point3d(100, 200, 0));
            
            // Draw filter chain
            DrawFilterChain(btr, new Point3d(300, 150, 0));
            
            // Draw comparator network
            DrawComparatorNetwork(btr, new Point3d(500, 150, 0));
            
            // Draw summing amplifier
            DrawSummingAmplifier(btr, new Point3d(700, 150, 0));

            trans.Commit();
        }
    }

    private void CreateCircuitLayers(Database db)
    {
        using (Transaction trans = db.TransactionManager.StartTransaction())
        {
            LayerTable lt = trans.GetObject(db.LayerTableId, OpenMode.ForRead) as LayerTable;
            
            if (!lt.Has(CIRCUIT_LAYER))
            {
                LayerTableRecord ltr = new LayerTableRecord();
                ltr.Name = CIRCUIT_LAYER;
                ltr.Color = Autodesk.AutoCAD.Colors.Color.FromRgb(0, 0, 255);
                
                lt.UpgradeOpen();
                lt.Add(ltr);
                trans.AddNewlyCreatedDBObject(ltr, true);
            }

            if (!lt.Has(TEXT_LAYER))
            {
                LayerTableRecord ltr = new LayerTableRecord();
                ltr.Name = TEXT_LAYER;
                ltr.Color = Autodesk.AutoCAD.Colors.Color.FromRgb(0, 255, 0);
                
                lt.UpgradeOpen();
                lt.Add(ltr);
                trans.AddNewlyCreatedDBObject(ltr, true);
            }

            trans.Commit();
        }
    }

    private void DrawPowerRails(BlockTableRecord btr)
    {
        // +15V Rail
        Line posRail = new Line(
            new Point3d(50, 50, 0),
            new Point3d(1150, 50, 0)
        );
        posRail.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(posRail);

        // -15V Rail
        Line negRail = new Line(
            new Point3d(50, 750, 0),
            new Point3d(1150, 750, 0)
        );
        negRail.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(negRail);

        // Ground Bus
        Line gndRail = new Line(
            new Point3d(50, 400, 0),
            new Point3d(1150, 400, 0)
        );
        gndRail.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(gndRail);

        // Add labels
        AddText(btr, "+15V", new Point3d(20, 55, 0));
        AddText(btr, "-15V", new Point3d(20, 755, 0));
        AddText(btr, "GND", new Point3d(20, 405, 0));
    }

    private void DrawTemperatureSensor(BlockTableRecord btr, Point3d basePoint)
    {
        // Draw Wheatstone bridge
        DrawWheatstoneCircuit(btr, basePoint);
        
        // Draw instrumentation amplifier
        DrawOpAmp(btr, basePoint.Add(new Vector3d(80, 20, 0)), "AD620");
        
        // Add component values
        AddText(btr, "10kΩ NTC", basePoint.Add(new Vector3d(40, 25, 0)));
        AddText(btr, "G=100", basePoint.Add(new Vector3d(85, 50, 0)));
    }

    private void DrawPressureSensor(BlockTableRecord btr, Point3d basePoint)
    {
        // Draw pressure sensor symbol
        Rectangle3d sensorRect = new Rectangle3d(
            basePoint,
            Vector3d.XAxis,
            Vector3d.YAxis,
            40, 60
        );
        Polyline sensorOutline = new Polyline();
        sensorOutline.CreateRectangle(sensorRect);
        sensorOutline.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(sensorOutline);

        // Add sensor label
        AddText(btr, "MPX5100", basePoint.Add(new Vector3d(5, 30, 0)));
        
        // Draw differential amplifier
        DrawOpAmp(btr, basePoint.Add(new Vector3d(60, 20, 0)), "OP27");
    }

    private void DrawFilterChain(BlockTableRecord btr, Point3d basePoint)
    {
        // Draw first stage filter
        DrawActiveFilter(btr, basePoint, "1kHz LP");
        
        // Draw second stage filter
        DrawActiveFilter(btr, basePoint.Add(new Vector3d(100, 0, 0)), "100Hz LP");
        
        // Add component values
        AddText(btr, "10kΩ", basePoint.Add(new Vector3d(-5, 25, 0)));
        AddText(btr, "100nF", basePoint.Add(new Vector3d(20, 55, 0)));
    }

    private void DrawComparatorNetwork(BlockTableRecord btr, Point3d basePoint)
    {
        // Draw upper comparator
        DrawComparator(btr, basePoint, "LM339");
        
        // Draw lower comparator
        DrawComparator(btr, basePoint.Add(new Vector3d(0, 80, 0)), "LM339");
        
        // Draw reference voltage source
        DrawVoltageReference(btr, basePoint.Add(new Vector3d(60, 40, 0)), "LM399\n6.95V");
    }

    private void DrawSummingAmplifier(BlockTableRecord btr, Point3d basePoint)
    {
        // Draw op-amp symbol
        DrawOpAmp(btr, basePoint, "OP477");
        
        // Draw input resistors
        DrawResistor(btr, basePoint.Add(new Vector3d(-30, 10, 0)), "20kΩ");
        
        // Draw feedback network
        DrawResistor(btr, basePoint.Add(new Vector3d(40, 10, 0)), "20kΩ");
    }

    private void DrawOpAmp(BlockTableRecord btr, Point3d basePoint, string label)
    {
        // Draw op-amp triangle
        Point3dCollection points = new Point3dCollection
        {
            basePoint,
            basePoint.Add(new Vector3d(0, 60, 0)),
            basePoint.Add(new Vector3d(40, 30, 0))
        };
        
        Polyline opAmp = new Polyline();
        opAmp.CreatePolyline(points);
        opAmp.Closed = true;
        opAmp.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(opAmp);

        // Add label
        AddText(btr, label, basePoint.Add(new Vector3d(10, 35, 0)));
    }

    private void AddText(BlockTableRecord btr, string text, Point3d position)
    {
        DBText dbText = new DBText();
        dbText.Position = position;
        dbText.Height = TEXT_HEIGHT;
        dbText.TextString = text;
        dbText.Layer = TEXT_LAYER;
        btr.AppendEntity(dbText);
    }

    private void DrawResistor(BlockTableRecord btr, Point3d basePoint, string value)
    {
        // Draw resistor symbol
        Point3dCollection points = new Point3dCollection
        {
            basePoint,
            basePoint.Add(new Vector3d(20, 0, 0))
        };
        
        Polyline resistor = new Polyline();
        resistor.CreatePolyline(points);
        resistor.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(resistor);

        // Add value
        AddText(btr, value, basePoint.Add(new Vector3d(5, -5, 0)));
    }

    private void DrawWheatstoneCircuit(BlockTableRecord btr, Point3d basePoint)
    {
        // Draw bridge outline
        Point3dCollection points = new Point3dCollection
        {
            basePoint,
            basePoint.Add(new Vector3d(50, 0, 0)),
            basePoint.Add(new Vector3d(25, 50, 0))
        };
        
        Polyline bridge = new Polyline();
        bridge.CreatePolyline(points);
        bridge.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(bridge);
    }

    private void DrawActiveFilter(BlockTableRecord btr, Point3d basePoint, string label)
    {
        // Draw op-amp
        DrawOpAmp(btr, basePoint, "OP27");
        
        // Draw RC network
        DrawResistor(btr, basePoint.Add(new Vector3d(-30, 20, 0)), "10kΩ");
        
        // Draw capacitor
        Line capLine = new Line(
            basePoint.Add(new Vector3d(10, 30, 0)),
            basePoint.Add(new Vector3d(10, 50, 0))
        );
        capLine.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(capLine);
        
        // Add filter label
        AddText(btr, label, basePoint.Add(new Vector3d(0, -10, 0)));
    }

    private void DrawVoltageReference(BlockTableRecord btr, Point3d basePoint, string label)
    {
        // Draw reference symbol
        Rectangle3d refRect = new Rectangle3d(
            basePoint,
            Vector3d.XAxis,
            Vector3d.YAxis,
            40, 60
        );
        Polyline refOutline = new Polyline();
        refOutline.CreateRectangle(refRect);
        refOutline.Layer = CIRCUIT_LAYER;
        btr.AppendEntity(refOutline);

        // Add label
        AddText(btr, label, basePoint.Add(new Vector3d(5, 30, 0)));
    }
}

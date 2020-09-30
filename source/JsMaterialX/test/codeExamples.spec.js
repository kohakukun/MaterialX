import { expect } from 'chai';
import { initMaterialX, getMtlxStrings } from './testHelpers';

describe('Code Examples', () => {
    it('Building a MaterialX Document', async () => {
        const mx = await initMaterialX();
        // Create a document.
        const doc = mx.createDocument();

        // Create a node graph with a single image node and output.
        const nodeGraph = doc.addNodeGraph();
        expect(doc.getNodeGraphs().length).to.equal(1);
        const image = nodeGraph.addNode('image');
        const nodes = nodeGraph.getNodes();
        expect(nodes.length).to.equal(1);
        expect(nodes[0]).to.eql(image);

        image.setParameterValuestring('file', 'image1.tif', 'filename');
        const param = image.getParameter('file');
        expect(param).to.not.be.null;
        expect(param.getValue().getData()).to.equal('image1.tif');

        const output = nodeGraph.addOutput();
        const outputs = nodeGraph.getOutputs();
        expect(outputs.length).to.equal(1);
        expect(outputs[0]).to.eql(output);

        output.setConnectedNode(image);
        const connectedNode = output.getConnectedNode();
        expect(connectedNode).to.not.be.null;
        expect(connectedNode instanceof mx.Node).to.be.true;

        // Create a simple shader interface.
        const simpleSrf = doc.addNodeDef('ND_simpleSrf', 'surfaceshader', 'simpleSrf');
        const nodeDefs = doc.getNodeDefs();
        expect(nodeDefs.length).to.equal(1);
        expect(nodeDefs[0]).to.eql(simpleSrf);

        simpleSrf.setInputValuecolor3('diffColor', new mx.Color3(1.0, 1.0, 1.0));
        let inputValue = simpleSrf.getInputValue('diffColor');
        expect(inputValue).to.not.be.null;
        expect(inputValue.getData()).to.eql(new mx.Color3(1.0, 1.0, 1.0));

        simpleSrf.setInputValuecolor3('specColor', new mx.Color3(0.0, 0.0, 0.0));
        inputValue = simpleSrf.getInputValue('specColor');
        expect(inputValue).to.not.be.null;
        expect(inputValue.getData()).to.eql(new mx.Color3(0.0, 0.0, 0.0));

        const roughness = simpleSrf.setParameterValuefloat('roughness', 0.25);
        const paramValue = simpleSrf.getParameterValue('roughness');
        expect(paramValue).to.not.be.null;
        expect(paramValue.getData()).to.equal(0.25);

        // Create a material that instantiates the shader.
        const material = doc.addMaterial();
        const materials = doc.getMaterials();
        expect(materials.length).to.equal(1);
        expect(materials[0]).to.eql(material);
        const refSimpleSrf = material.addShaderRef('SR_simpleSrf', 'simpleSrf');
        const shaderRefs = material.getShaderRefs();
        expect(shaderRefs.length).to.equal(1);
        expect(shaderRefs[0]).to.eql(refSimpleSrf);
        expect(shaderRefs[0].getName()).to.equal('SR_simpleSrf');

        // Bind roughness to a new value within this material.
        const bindParam = refSimpleSrf.addBindParam('roughness');
        const bindParams = refSimpleSrf.getBindParams();
        expect(bindParams.length).to.equal(1);
        expect(bindParams[0]).to.eql(bindParam);
        bindParam.setValuefloat(0.5);
        expect(bindParam.getValue()).to.not.be.null;
        expect(bindParam.getValue().getData()).to.equal(0.5);

        // Validate the value of roughness in the context of this material.
        expect(roughness.getBoundValue(material).getValueString()).to.equal('0.5');
    });

    it('Traversing a Document Tree', async () => {
        const xmlStr = getMtlxStrings(
            ['standard_surface_greysphere_calibration.mtlx'],
            '../../../resources/Materials/Examples/StandardSurface'
        )[0];
        const mx = await initMaterialX();

        // Read a document from disk.
        const doc = mx.createDocument();
        mx.readFromXmlString(doc, xmlStr);

        // Traverse the document tree in depth-first order.
        const elements = doc.traverseTree();
        let elem = elements.next();
        expect(elem).to.exist;
        let elementCount = 0;
        let nodeCount = 0;
        let fileCount = 0;
        while (elem) {
            elementCount++;
            // Display the filename of each image node.
            if (elem instanceof mx.Node) {
                nodeCount++;
                const param = elem.getParameter('file');
                if (param) {
                    fileCount++;
                    const filename = param.getValueString();
                    expect(elem.getName()).to.equal('image1');
                    expect(filename).to.equal('greysphere_calibration.png');
                }
            }
            elem = elements.next();
        }
        expect(elementCount).to.equal(23);
        expect(nodeCount).to.equal(5);
        expect(fileCount).to.equal(1);
    });

    it('Building a MaterialX Document', async () => {
        const xmlStr = getMtlxStrings(['MaterialBasic.mtlx'], '../../../resources/Materials/Examples/Syntax')[0];
        const mx = await initMaterialX();

        // Read a document from disk.
        const doc = mx.createDocument();
        mx.readFromXmlString(doc, xmlStr);

        let materialCount = 0;
        let shaderParamCount = 0;
        let shaderInputCount = 0;
        // Iterate through 1.37 materials for which there should be none
        const materials = doc.getMaterials();
        materials.forEach((material) => {
            materialCount++;
            // For each shader parameter, compute its value in the context of this material.
            const primaryShaderParams = material.getPrimaryShaderParameters();
            primaryShaderParams.forEach((param) => {
                const value = param.getBoundValue(material);
                expect(value instanceof mx.TypedValue_float).to.be.true;
                expect(value.getTypeString()).to.equal('float');
                expect(param.getName()).to.equal('fresnel_exp');
                shaderParamCount++;
            });

            // For each shader input, find all upstream images in the dataflow graph.
            const primaryShaderInputs = material.getPrimaryShaderInputs();
            primaryShaderInputs.forEach((input) => {
                const graphIter = input.traverseGraph(material);
                let edge = graphIter.next();
                while (edge) {
                    shaderInputCount++;
                    edge = graphIter.next();
                }
            });
        });

        expect(materialCount).to.equal(0);
        expect(shaderParamCount).to.equal(0);
        expect(shaderInputCount).to.equal(0);
    });
});
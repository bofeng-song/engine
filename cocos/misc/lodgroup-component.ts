/*
 Copyright (c) 2022 Xiamen Yaji Software Co., Ltd.

 https://www.cocos.com/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

import { ccclass, editable, executeInEditMode, menu, serializable, type } from 'cc.decorator';
import { Mesh, MeshRenderer } from '../3d';
import { Vec3 } from '../core/math';
import { scene } from '../render-scene';
import { Component } from '../scene-graph/component';

const _DEFAULT_SCREEN_OCCUPATION: number[] = [0.5, 0.2, 0.07];
@ccclass('cc.LOD')
export class LOD {
    // The relative minimum transition height in screen space.
    @serializable
    protected _screenRelativeTransitionHeight = 1;
    // Mesh renderers components contained in this LOD level.
    @type([MeshRenderer])
    @serializable
    protected _renderers: (MeshRenderer | null)[] = [];
    // renderer internal LOD data block.
    protected _LOD: scene.LOD = new scene.LOD();

    constructor () {
        this._LOD.screenRelativeTransitionHeight = this._screenRelativeTransitionHeight;
    }

    /**
     * @en The relvative (minimum) transition height of this LOD level in screen space
     * @zh 本层级（最小）相对屏幕区域的过渡高度
     */
    @type(Number)
    get screenRelativeTransitionHeight () { return this._screenRelativeTransitionHeight; }
    set screenRelativeTransitionHeight (val) {
        this._screenRelativeTransitionHeight = val;
        this._LOD.screenRelativeTransitionHeight = val;
    }

    @type([MeshRenderer])
    get renderers () {
        return this._renderers;
    }

    set renderers (meshList) {
        this._renderers = meshList;
    }

    @editable
    @type([Number])
    get triangles () {
        const tris: number[] = [];
        this._renderers.forEach((meshRenderer: MeshRenderer | null) => {
            let count = 0;
            if (meshRenderer && meshRenderer.mesh) {
                const primitives = meshRenderer.mesh.struct.primitives;
                primitives?.forEach((subMesh: Mesh.ISubMesh) => {
                    if (subMesh && subMesh.indexView) {
                        count += subMesh.indexView.count;
                    }
                });
            }
            tris.push(count / 3);
        });
        return tris;
    }

    /**
     * @en Insert a [[MeshRenderer]] before specific index position.
     * @zh 在指定的数组索引处插入一个[[MeshRenderer]]
     * @param index 0 indexed position in renderer array, when -1 is specified, append to the tail of the list
     * @param renderer the mesh-renderer object
     * @returns The renderer inserted
     */
    insertRenderer (index: number, renderer: MeshRenderer): MeshRenderer {
        this._renderers.splice(index, 0, renderer);
        this._LOD.models.splice(index, 0, renderer.model!);
        return renderer;
    }

    /**
     * @en Delete the [[MeshRenderer]] at specific index position.
     * @zh 删除指定索引处的[[MeshRenderer]]
     * @param index 0 indexed position in renderer array, when -1 is specified, the last element will be deleted
     * @returns The renderer deleted
     */
    deleteRenderer (index: number): MeshRenderer | null {
        const renderer = this._renderers[index];
        this._renderers.splice(index, 1);
        this._LOD.models.splice(index, 1);
        return renderer;
    }

    getRenderer (index: number): MeshRenderer | null {
        return this._renderers[index];
    }

    setRenderer (index: number, renderer: MeshRenderer) {
        this._renderers[index] = renderer;
        this._LOD.models[index] = renderer.model!;
    }

    get rendererCount () { return this._renderers.length; }

    get lod () { return this._LOD; }
}

@ccclass('cc.LODGroup')
@menu('Rendering/LOD Group')
@executeInEditMode
export class LODGroup extends Component {
    /**
     * @en Object reference point in local space, e.g. center of the bound volume for all LODs
     */
    @serializable
    protected _localReferencePoint: Vec3 = new Vec3(0, 0, 0);

    /**
     * @en Object Size in local space, may be auto-calculated value from object bounding box or value from user input.
     */
    @serializable
    protected _objectSize = 1;

    /**
     *@en The array of LODs
     */
    @serializable
    protected _LODs: LOD[] = [];

    protected _lodGroup = new scene.LODGroup();

    constructor () {
        super();
        this._lodGroup.objectSize = this._objectSize;
    }

    set localReferencePoint (val: Vec3) {
        this._localReferencePoint.set(val);
        this._lodGroup.localReferencePoint = val;
    }

    get localReferencePoint () { return this._localReferencePoint.clone(); }

    get lodCount () { return this._LODs.length; }

    @type(Number)
    set objectSize (val: number) {
        this._objectSize = val;
        this._lodGroup.objectSize = val;
    }

    get objectSize () { return this._objectSize; }

    @type([LOD])
    get LODs () {
        return this._LODs;
    }

    set LODs (LODs: LOD[]) {
        this._LODs = LODs;
    }

    insertLOD (index: number, lod: LOD): LOD {
        this._LODs.splice(index, 0, lod);
        this._lodGroup.LODs.splice(index, 0, lod.lod);
        return lod;
    }

    deleteLOD (index: number) : LOD {
        const lod = this._LODs[index];
        this._LODs.splice(index, 1);
        this._lodGroup.LODs.splice(index, 1);
        return lod;
    }

    getLOD (index: number): LOD {
        return this._LODs[index];
    }

    setLOD (index: number, lod: LOD) {
        this._LODs[index] = lod;
    }

    get lodGroup () { return this._lodGroup; }

    onLoad () {
        this._lodGroup.node = this.node;

        // generate default lod for lodGroup
        if (this.lodCount < 1) {
            const size = _DEFAULT_SCREEN_OCCUPATION.length;
            for (let i = 0; i < size; i++) {
                const lod = new LOD();
                lod.screenRelativeTransitionHeight = _DEFAULT_SCREEN_OCCUPATION[i];
                this.insertLOD(i, lod);
            }
        }
    }

    // Redo, Undo, Prefab restore, etc.
    onRestore () {
        if (this.enabledInHierarchy) {
            this._attachToScene();
        }
    }

    onEnable () {
        this._attachToScene();
        //   LODGroupEditorUtility.recalculateBounds(this);

        // lod's model will be enabled while execute culling
        for (const lod of this._LODs) {
            for (const renderer of lod.renderers) {
                if (!renderer) {
                    continue;
                }
                const renderScene = renderer._getRenderScene();
                if (renderScene && renderer.model) {
                    renderScene.removeModel(renderer.model);
                }
            }
        }

        // cache lod for scene
        if (this.lodCount > 0 && this._lodGroup.lodCount < 1) {
            this._LODs.forEach((lod: LOD, index) => {
                lod.lod.screenRelativeTransitionHeight = lod.screenRelativeTransitionHeight;
                const renderers = lod.renderers;
                if (renderers !== null && renderers.length > 0) {
                    for (let i = 0; i < renderers.length; i++) {
                        const lodInstance = lod.lod;
                        const renderer = renderers[i];
                        if (lodInstance && renderer && renderer.model) {
                            lodInstance.models[i] = renderer.model;
                        }
                    }
                }
                this._lodGroup.LODs[index] = lod.lod;
            });
        }
    }

    onDisable () {
        this._detachFromScene();
    }

    protected _attachToScene () {
        if (!this.node.scene) { return; }

        const renderScene = this._getRenderScene();
        if (this._lodGroup.scene) {
            this._detachFromScene();
        }
        renderScene.addLODGroup(this._lodGroup);
    }

    protected _detachFromScene () {
        if (this._lodGroup.scene) { this._lodGroup.scene.removeLODGroup(this._lodGroup); }
    }
}

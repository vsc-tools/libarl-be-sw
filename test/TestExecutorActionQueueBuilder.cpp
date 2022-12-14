/*
 * TestExecutorActionQueueBuilder.cpp
 *
 * Copyright 2022 Matthew Ballance and Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may 
 * not use this file except in compliance with the License.  
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, Gsoftware 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * Created on:
 *     Author:
 */
#include <vector>
#include "ArlImpl.h"
#include "arl/IContext.h"
#include "vsc/impl/ModelBuildContext.h"
#include "TaskBuildExecutorActionQueues.h"
#include "TestExecutorActionQueueBuilder.h"


namespace arl {
namespace be {
namespace sw {


TestExecutorActionQueueBuilder::TestExecutorActionQueueBuilder() {

}

TestExecutorActionQueueBuilder::~TestExecutorActionQueueBuilder() {

}

TEST_F(TestExecutorActionQueueBuilder, smoke) {
    std::vector<arl::IModelActivityUP>      activities_u;
    std::vector<arl::IModelActivity *>      activities;
    std::vector<arl::IModelFieldActionUP>   actions;

    for (uint32_t i=0; i<16; i++) {
        IModelFieldAction *action = m_arl_ctxt->mkModelFieldActionRoot("a", 0);
        actions.push_back(IModelFieldActionUP(action));
        IModelActivityTraverse *t = m_arl_ctxt->mkModelActivityTraverse(
            action,
            0);
        activities_u.push_back(IModelActivityUP(t));
        activities.push_back(t);
    }

    std::vector<ExecutorActionQueue> queues;
    TaskBuildExecutorActionQueues(m_arl_ctxt.get(), {}, -1).build(
        queues,
        activities
    );

    ASSERT_EQ(queues.size(), 1);
    ASSERT_EQ(queues[0].size(), 16);
}

TEST_F(TestExecutorActionQueueBuilder, seq_alt_executors) {
    std::vector<arl::IModelActivityUP>      activities_u;
    std::vector<arl::IModelActivity *>      activities;
    std::vector<arl::IModelFieldActionUP>   actions;

    m_arl_ctxt->getDebugMgr()->enable(true);

    vsc::IDataTypeStruct *claim_t = m_arl_ctxt->mkDataTypeStruct("claim_t");
    m_arl_ctxt->addDataTypeStruct(claim_t);

    arl::IDataTypeComponent *comp_t = m_arl_ctxt->mkDataTypeComponent("comp_t");
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec1", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec2", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec3", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec4", claim_t, false));
    m_arl_ctxt->addDataTypeComponent(comp_t);


    // Use a data type in order to get a claim
    IDataTypeAction *action_t = m_arl_ctxt->mkDataTypeAction("action_t");
    action_t->addField(m_arl_ctxt->mkTypeFieldExecutorClaim("claim", claim_t, false));
    action_t->setComponentType(comp_t);
    m_arl_ctxt->addDataTypeAction(action_t);

    vsc::ModelBuildContext build_ctxt(m_arl_ctxt.get());
    IModelFieldComponent *comp = comp_t->mkRootFieldT<IModelFieldComponent>(
        &build_ctxt,
        "pss_top",
        false);
    IModelFieldExecutor *exec1 = comp->getFieldT<IModelFieldExecutor>(0);
    IModelFieldExecutor *exec2 = comp->getFieldT<IModelFieldExecutor>(1);
    IModelFieldExecutor *exec3 = comp->getFieldT<IModelFieldExecutor>(2);
    IModelFieldExecutor *exec4 = comp->getFieldT<IModelFieldExecutor>(3);

    for (uint32_t i=0; i<16; i++) {
        IModelFieldAction *action = action_t->mkRootFieldT<IModelFieldAction>(
            &build_ctxt, "a", false);
        IModelFieldExecutorClaim *claim = action->getFieldT<IModelFieldExecutorClaim>(1);
        claim->setRef((i%2)?exec2:exec1);
        actions.push_back(IModelFieldActionUP(action));
        IModelActivityTraverse *t = m_arl_ctxt->mkModelActivityTraverse(
            action,
            0);
        activities_u.push_back(IModelActivityUP(t));
        activities.push_back(t);
    }

    std::vector<IModelFieldExecutor *> executors({exec1, exec2, exec3, exec4});

    std::vector<ExecutorActionQueue> queues;
    TaskBuildExecutorActionQueues(m_arl_ctxt.get(), executors, 0).build(
        queues,
        activities
    );

    ASSERT_EQ(queues.size(), 4);
    // Expect the primary executor to have
    // - 1 entry publishing its readiness
    // - 15 entries executing actions and waiting for its peer
    // - 3 entries synchronizing with core termination
    ASSERT_EQ(queues[0].size(), 19);
    // Expect the non-primary peer core to have
    // - 1 entry waiting for the primary core 
    // - 16 entries synchronizing
    // - 1 entry notifying the primary core 
    ASSERT_EQ(queues[1].size(), 18); 
    // Expect these idle cores to have
    // - 1 entry waiting for the primary core
    // - 1 entry notifying the primary core
    ASSERT_EQ(queues[2].size(), 2);
    ASSERT_EQ(queues[3].size(), 2);
}

TEST_F(TestExecutorActionQueueBuilder, seq_executors) {
    std::vector<arl::IModelActivityUP>      activities_u;
    std::vector<arl::IModelActivity *>      activities;
    std::vector<arl::IModelFieldActionUP>   actions;

    m_arl_ctxt->getDebugMgr()->enable(true);

    vsc::IDataTypeStruct *claim_t = m_arl_ctxt->mkDataTypeStruct("claim_t");
    m_arl_ctxt->addDataTypeStruct(claim_t);

    arl::IDataTypeComponent *comp_t = m_arl_ctxt->mkDataTypeComponent("comp_t");
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec1", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec2", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec3", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec4", claim_t, false));
    m_arl_ctxt->addDataTypeComponent(comp_t);


    // Use a data type in order to get a claim
    IDataTypeAction *action_t = m_arl_ctxt->mkDataTypeAction("action_t");
    action_t->addField(m_arl_ctxt->mkTypeFieldExecutorClaim("claim", claim_t, false));
    action_t->setComponentType(comp_t);
    m_arl_ctxt->addDataTypeAction(action_t);

    vsc::ModelBuildContext build_ctxt(m_arl_ctxt.get());
    IModelFieldComponent *comp = comp_t->mkRootFieldT<IModelFieldComponent>(
        &build_ctxt,
        "pss_top",
        false);
    IModelFieldExecutor *exec1 = comp->getFieldT<IModelFieldExecutor>(0);
    IModelFieldExecutor *exec2 = comp->getFieldT<IModelFieldExecutor>(1);
    IModelFieldExecutor *exec3 = comp->getFieldT<IModelFieldExecutor>(2);
    IModelFieldExecutor *exec4 = comp->getFieldT<IModelFieldExecutor>(3);

    for (uint32_t i=0; i<16; i++) {
        IModelFieldAction *action = action_t->mkRootFieldT<IModelFieldAction>(
            &build_ctxt, "a", false);
        IModelFieldExecutorClaim *claim = action->getFieldT<IModelFieldExecutorClaim>(1);
        claim->setRef((i<8)?exec1:exec2);
        actions.push_back(IModelFieldActionUP(action));
        IModelActivityTraverse *t = m_arl_ctxt->mkModelActivityTraverse(
            action,
            0);
        activities_u.push_back(IModelActivityUP(t));
        activities.push_back(t);
    }

    std::vector<IModelFieldExecutor *> executors({exec1, exec2, exec3, exec4});

    std::vector<ExecutorActionQueue> queues;
    TaskBuildExecutorActionQueues(m_arl_ctxt.get(), executors, 0).build(
        queues,
        activities
    );

    ASSERT_EQ(queues.size(), 4);
    // The primary executor has
    // - 1 initial notification
    // - 8 synchronizations and executions
    // - 3 final depends
    ASSERT_EQ(queues[0].size(), 12);
    // The non-primary executor has
    // - 1 initial depend
    // - 9 synchronizations and executions
    // - 1 final notification
    ASSERT_EQ(queues[1].size(), 11); 
    ASSERT_EQ(queues[2].size(), 2);
    ASSERT_EQ(queues[3].size(), 2);
}

TEST_F(TestExecutorActionQueueBuilder, seq_par_diff_executor) {
    std::vector<arl::IModelActivityUP>      activities_u;
    std::vector<arl::IModelActivity *>      activities;
    std::vector<arl::IModelFieldActionUP>   actions;

    m_arl_ctxt->getDebugMgr()->enable(true);

    vsc::IDataTypeStruct *claim_t = m_arl_ctxt->mkDataTypeStruct("claim_t");
    m_arl_ctxt->addDataTypeStruct(claim_t);

    arl::IDataTypeComponent *comp_t = m_arl_ctxt->mkDataTypeComponent("comp_t");
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec1", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec2", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec3", claim_t, false));
    comp_t->addField(m_arl_ctxt->mkTypeFieldExecutor("exec4", claim_t, false));
    m_arl_ctxt->addDataTypeComponent(comp_t);


    // Use a data type in order to get a claim
    IDataTypeAction *action_t = m_arl_ctxt->mkDataTypeAction("action_t");
    action_t->addField(m_arl_ctxt->mkTypeFieldExecutorClaim("claim", claim_t, false));
    action_t->setComponentType(comp_t);
    m_arl_ctxt->addDataTypeAction(action_t);

    vsc::ModelBuildContext build_ctxt(m_arl_ctxt.get());
    IModelFieldComponent *comp = comp_t->mkRootFieldT<IModelFieldComponent>(
        &build_ctxt,
        "pss_top",
        false);
    IModelFieldExecutor *exec1 = comp->getFieldT<IModelFieldExecutor>(0);
    IModelFieldExecutor *exec2 = comp->getFieldT<IModelFieldExecutor>(1);
    IModelFieldExecutor *exec3 = comp->getFieldT<IModelFieldExecutor>(2);
    IModelFieldExecutor *exec4 = comp->getFieldT<IModelFieldExecutor>(3);

    for (uint32_t i=0; i<16; i++) {
        IModelFieldAction *action = action_t->mkRootFieldT<IModelFieldAction>(
            &build_ctxt, "a", false);
        IModelFieldExecutorClaim *claim = action->getFieldT<IModelFieldExecutorClaim>(1);
        claim->setRef((i<8)?exec1:exec2);
        actions.push_back(IModelFieldActionUP(action));
        IModelActivityTraverse *t = m_arl_ctxt->mkModelActivityTraverse(
            action,
            0);
        activities_u.push_back(IModelActivityUP(t));
        activities.push_back(t);
    }

    std::vector<IModelFieldExecutor *> executors({exec1, exec2, exec3, exec4});

    std::vector<ExecutorActionQueue> queues;
    TaskBuildExecutorActionQueues(m_arl_ctxt.get(), executors, 0).build(
        queues,
        activities
    );

    ASSERT_EQ(queues.size(), 4);
    ASSERT_EQ(queues[0].size(), 8);
    ASSERT_EQ(queues[1].size(), 9); 
    ASSERT_EQ(queues[2].size(), 0);
    ASSERT_EQ(queues[3].size(), 0);
}

}
}
}

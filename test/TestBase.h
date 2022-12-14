/**
 * TestBase.h
 *
 * Copyright 2022 Matthew Ballance and Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may 
 * not use this file except in compliance with the License.  
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 *
 * Created on:
 *     Author: 
 */
#pragma once
#include "gtest/gtest.h"
#include "arl/IContext.h"
#include "vsc/IContext.h"

namespace arl {
namespace be {
namespace sw {


class TestBase : public ::testing::Test {
public:
    TestBase();

    virtual ~TestBase();

    virtual void SetUp() override;

    virtual void TearDown() override;

protected:
    vsc::IContext               *m_vsc_ctxt;
    arl::IContextUP             m_arl_ctxt;

};

}
}
}



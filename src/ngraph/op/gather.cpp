//*****************************************************************************
// Copyright 2017-2019 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include "ngraph/op/gather.hpp"
#include "ngraph/op/constant.hpp"
#include "ngraph/shape.hpp"

using namespace std;
using namespace ngraph;

static int PARAMS = 0;
static int INDICES = 1;
static int AXIS = 2;

const string op::Gather::type_name{"Gather"};

op::Gather::Gather(const Output<Node>& params, const Output<Node>& indices, size_t axis)
    : Op({params, indices, op::Constant::create(element::i64, Shape{1}, {axis})})
{
}

op::Gather::Gather(const Output<Node>& params,
                   const Output<Node>& indices,
                   const Output<Node>& axis)
    : Op({params, indices, axis})
{
    constructor_validate_and_infer_types();
}

size_t op::Gather::get_axis() const
{
    AxisVector axes;
    auto axes_input_node = input_value(AXIS).get_node_shared_ptr();
    if (auto const_op = dynamic_pointer_cast<op::Constant>(axes_input_node))
    {
        axes = const_op->get_axis_vector_val();
    }
    NODE_VALIDATION_CHECK(
        this, axes.size() == 1, "Axes must have 1 element (axes.size: ", axes.size(), ").");
    return axes.front();
}

shared_ptr<Node> op::Gather::copy_with_new_args(const NodeVector& new_args) const
{
    check_new_args_count(this, new_args);
    return make_shared<Gather>(new_args.at(PARAMS), new_args.at(INDICES), new_args.at(AXIS));
}

void op::Gather::validate_and_infer_types()
{
    const auto& axis_shape = get_input_partial_shape(AXIS);
    NODE_VALIDATION_CHECK(this,
                          static_cast<size_t>(axis_shape.rank()) == 1,
                          "Axis for padding value is not a scalar (shape: ",
                          axis_shape,
                          ").");

    element::Type result_et = get_input_element_type(PARAMS);
    element::Type indices_et = get_input_element_type(INDICES);

    const PartialShape& params_shape = get_input_partial_shape(PARAMS);
    const PartialShape& indices_shape = get_input_partial_shape(INDICES);

    NODE_VALIDATION_CHECK(this,
                          indices_et == element::i32 || indices_et == element::i64,
                          "Indices element type must be i64 or i32");

    // params rank must be at least (axis + 1)
    // indices value must be in range [0, params.shape[axis]).
    // output rank is rank(params) + rank(indices) - 1
    const auto axis = get_axis();
    NODE_VALIDATION_CHECK(this,
                          params_shape.rank().is_dynamic() ||
                              static_cast<size_t>(params_shape.rank()) > static_cast<size_t>(axis),
                          "params rank is expected to be at least axis + 1");

    PartialShape result_shape;
    if (params_shape.rank().is_static() && indices_shape.rank().is_static())
    {
        std::vector<Dimension> result_dims(static_cast<size_t>(params_shape.rank()) +
                                           static_cast<size_t>(indices_shape.rank()) - 1);
        size_t i = 0;
        for (; i < static_cast<size_t>(axis); i++)
        {
            result_dims[i] = params_shape[i];
        }
        for (size_t j = 0; j < static_cast<size_t>(indices_shape.rank()); i++, j++)
        {
            result_dims[i] = indices_shape[j];
        }
        for (size_t j = static_cast<size_t>(axis) + 1; j < static_cast<size_t>(params_shape.rank());
             i++, j++)
        {
            result_dims[i] = params_shape[j];
        }

        result_shape = PartialShape(result_dims);
    }
    else
    {
        result_shape = PartialShape::dynamic();
    }

    set_output_type(0, result_et, result_shape);
}

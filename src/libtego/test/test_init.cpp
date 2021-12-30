#include <catch2/catch.hpp>
#include <tego/tego.h>
#include <tego/tego.hpp>

TEST_CASE(  "Context can be created/destroyed, with valid inputs",
            "[libtego][context][init][deinit][valid_input]")
{
    tego_context* context = nullptr;

    // require that the context is successfully initialized
    REQUIRE_NOTHROW(tego_initialize(&context, tego::throw_on_error()));
    REQUIRE(context != nullptr);

    // require that the uninitialization is successful
    REQUIRE_NOTHROW(tego_uninitialize(context, tego::throw_on_error()));
}

TEST_CASE(  "Libtego refuses to create multiple contexts",
            "[libtego][context][init][invalid_input]")
{
    tego_context* context = nullptr;

    // the first context should be created successfully
    REQUIRE_NOTHROW(tego_initialize(&context, tego::throw_on_error()));
    REQUIRE(context != nullptr);

    // the second time should throw
    tego_context* context2 = nullptr;
    REQUIRE_THROWS(tego_initialize(&context2, tego::throw_on_error()));

    // clean up
    tego_uninitialize(context, nullptr);
}

TEST_CASE(  "Libtego refuses to create/destroy a context when passed nullptr",
            "[libtego][context][init][deinit][invalid_input]")
{
    // expect tego_initialize to throw when the first param is nullptr
    REQUIRE_THROWS(tego_initialize(nullptr, tego::throw_on_error()));

    // tego_uninitialize should do nothing when the first param is nullptr
    REQUIRE_NOTHROW(tego_uninitialize(nullptr, tego::throw_on_error()));

    // we should still be able to successfully create and destroy contexts now
    tego_context* context = nullptr;
    REQUIRE_NOTHROW(tego_initialize(&context, tego::throw_on_error()));
    REQUIRE_NOTHROW(tego_uninitialize(context, tego::throw_on_error()));
}

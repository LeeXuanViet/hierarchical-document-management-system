#pragma once

#include "app_context.hpp"

namespace dms {

/// Seed demo data (org units, users, documents) into the application context.
/// Only call this when the database is freshly created (isNewDatabase() == true).
void seedDemo(AppContext& app);

} // namespace dms

#pragma once

/// <summary>
/// class which represents a printer in the office.
/// </summary>
class printer {
  public:
    int printer_id;
    int current_company;
    int usage_count{0};
    int times_used{0};
};

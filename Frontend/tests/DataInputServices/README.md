# DataInputService Tests

This folder contains unit tests for the **DataInputService** module. The tests are implemented using the [Catch2](https://github.com/catchorg/Catch2) framework.

---

## How to Run

1. Make sure Catch2 is included in your project.
2. Build the test executable (using CMake or your preferred IDE).
3. Run the executable:

```bash
./FrontendServiceTest
```
4. The output will show the results of all test cases, including passed and failed sections.

## What Is Tested

### 1. Password Validation

**Function:** `DataInputService::passwordValid(QString password)`

**Tests cover:**
- Minimum and maximum length requirements.
- Invalid characters.
- Spaces inside or at the beginning of the password.
- Valid passwords.

### 2. Tag Validation

**Function:** `DataInputService::tagValid(QString tag)`

**Tests cover:**
- Empty tags.
- Tags shorter than the minimum allowed length.
- Tags starting with an underscore (`_`).
- Tags containing consecutive underscores (`__`).
- Tags containing dots (`.`).

### 3. Email Validation

**Function:** `DataInputService::emailValid(QString email)`

**Tests cover:**
- Emails without a domain.
- Emails with only a domain.
- Invalid characters in local or domain part.
- Local part length (too short or too long).
- Spaces in email (inside or in front).
- Valid emails.

### 4. Name Validation

**Function:** `DataInputService::nameValid(QString name)`

**Tests cover:**
- Empty names.
- Names shorter than the minimum allowed length.
- Names longer than the maximum allowed length.

---

## Notes

- Each test case is divided into `SECTION`s to isolate individual scenarios.
- Both positive (valid input) and negative (invalid input) cases are covered.
- Constants such as `kMaxPasswordLength` and `kMinEmailLocalPartLength` are used from `DataInputService::detail` to ensure tests reflect actual limits in the module.
